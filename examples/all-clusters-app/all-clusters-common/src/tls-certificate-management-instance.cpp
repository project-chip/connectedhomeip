/*
 *
 *    Copyright (c) 2025 Matter Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/tls-certificate-management-server/CertificateTableImpl.h>
#include <app/clusters/tls-certificate-management-server/tls-certificate-management-server.h>
#include <clusters/TlsCertificateManagement/Commands.h>
#include <crypto/CHIPCryptoPAL.h>
#include <tls-certificate-management-instance.h>
#include <tls-client-management-instance.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Tls;
using namespace chip::app::Clusters::TlsCertificateManagement;
using namespace chip::app::Clusters::TlsCertificateManagement::Structs;
using namespace Protocols::InteractionModel;
using namespace chip::Crypto;
using namespace chip::Platform;

static constexpr uint16_t kSpecMaxCertBytes     = 3000;
static constexpr uint16_t kMaxIntermediateCerts = 10;

struct InlineBufferedRootCert : CertificateTable::BufferedRootCert
{
    PersistentStore<CHIP_CONFIG_TLS_PERSISTED_ROOT_CERT_BYTES> buffer;
    InlineBufferedRootCert() : CertificateTable::BufferedRootCert(buffer) {}
};

struct InlineBufferedClientCert : CertificateTable::BufferedClientCert
{
    PersistentStore<CHIP_CONFIG_TLS_PERSISTED_CLIENT_CERT_BYTES> buffer;
    InlineBufferedClientCert() : CertificateTable::BufferedClientCert(buffer) {}
};

struct RefEncodableRootCert
{
    RefEncodableRootCert(TLSCertStruct::Type & cert) : certificate(&cert) {}
    RefEncodableRootCert() : certificate(nullptr) {}

    TLSCertStruct::Type * certificate;
    std::array<uint8_t, kSpecMaxCertBytes> certBytes;

    CHIP_ERROR FromPersistence(FabricIndex fabric, const CertificateTable::RootCertStruct & src)
    {
        VerifyOrReturnError(certificate != nullptr, CHIP_ERROR_INTERNAL);

        MutableByteSpan targetBytes(certBytes);
        ReturnErrorOnFailure(CopySpanToMutableSpan(src.certificate.Value(), targetBytes));

        certificate->fabricIndex = fabric;
        certificate->caid        = src.caid;
        certificate->certificate.SetValue(targetBytes);
        return CHIP_NO_ERROR;
    }
};

struct InlineEncodableRootCert : RefEncodableRootCert
{
    TLSCertStruct::Type inlineCertificate;

    InlineEncodableRootCert() : RefEncodableRootCert(inlineCertificate) {}
};

struct RefEncodableClientCert
{
    RefEncodableClientCert(TLSClientCertificateDetailStruct::Type & cert) : certificate(&cert) {}
    RefEncodableClientCert() : certificate(nullptr) {}

    TLSClientCertificateDetailStruct::Type * certificate;
    std::array<uint8_t, kSpecMaxCertBytes> certBytes;
    std::array<ByteSpan, kMaxIntermediateCerts> intermediateCerts;
    std::array<std::array<uint8_t, kSpecMaxCertBytes>, kMaxIntermediateCerts> intermediateCertBytes;

    CHIP_ERROR FromPersistence(FabricIndex fabric, const CertificateTable::ClientCertStruct & src)
    {
        VerifyOrReturnError(certificate != nullptr, CHIP_ERROR_INTERNAL);

        if (!src.clientCertificate.Value().IsNull())
        {
            MutableByteSpan targetBytes(certBytes);
            ReturnErrorOnFailure(CopySpanToMutableSpan(src.clientCertificate.Value().Value(), targetBytes));
            certificate->clientCertificate.SetValue(Nullable<ByteSpan>(targetBytes));
        }
        else
        {
            certificate->clientCertificate.SetValue(Nullable<ByteSpan>());
        }

        if (src.intermediateCertificates.HasValue())
        {
            auto srcIter = src.intermediateCertificates.Value().begin();
            uint8_t i    = 0;
            while (srcIter.Next())
            {
                VerifyOrReturnError(i < kMaxIntermediateCerts, CHIP_ERROR_NO_MEMORY);
                MutableByteSpan targetIntermediateBytes(intermediateCertBytes[i]);
                ReturnErrorOnFailure(CopySpanToMutableSpan(srcIter.GetValue(), targetIntermediateBytes));
                intermediateCerts[i++] = targetIntermediateBytes;
            }
            ReturnErrorOnFailure(srcIter.GetStatus());
            certificate->intermediateCertificates.SetValue(List<ByteSpan>(intermediateCerts.data(), i));
        }
        else
        {
            certificate->intermediateCertificates.SetValue(List<ByteSpan>());
        }

        certificate->fabricIndex = fabric;
        certificate->ccdid       = src.ccdid;
        return CHIP_NO_ERROR;
    }
};

struct InlineEncodableClientCert : RefEncodableClientCert
{
    TLSClientCertificateDetailStruct::Type inlineCertificate;

    InlineEncodableClientCert() : RefEncodableClientCert(inlineCertificate) {}
};

static constexpr uint8_t kMaxRootCerts   = kMaxRootCertificatesPerFabric;
static constexpr uint8_t kMaxClientCerts = kMaxClientCertificatesPerFabric;

CHIP_ERROR FingerprintMatch(const ByteSpan & fingerprint, const ByteSpan & cert, bool & outMatch)
{
    std::array<uint8_t, chip::Crypto::kSHA256_Hash_Length> fingerprintPayload = { 0 };
    MutableByteSpan calculatedFingerprint(fingerprintPayload);
    ReturnErrorOnFailure(Hash_SHA256(cert.data(), cert.size(), fingerprintPayload.data()));
    outMatch = fingerprint.data_equal(calculatedFingerprint);
    return CHIP_NO_ERROR;
}

Status TlsCertificateManagementCommandDelegate::ProvisionRootCert(EndpointId matterEndpoint, FabricIndex fabric,
                                                                  const ProvisionRootCertificateType & provisionReq,
                                                                  Tls::TLSCAID & outCaid)
{
    auto localId = provisionReq.caid.IsNull() ? Optional<Tls::TLSCAID>() : Optional<Tls::TLSCAID>(provisionReq.caid.Value());
    UniquePtr<InlineBufferedRootCert> certBuffer(New<InlineBufferedRootCert>());
    VerifyOrReturnError(certBuffer, Status::ResourceExhausted);

    auto result = mCertificateTable.UpsertRootCertificateEntry(fabric, localId, certBuffer->buffer, provisionReq.certificate);

    VerifyOrReturnValue(result == CHIP_NO_ERROR, Status::Failure);
    outCaid = localId.Value();
    return Status::Success;
}

CHIP_ERROR TlsCertificateManagementCommandDelegate::LoadedRootCerts(EndpointId matterEndpoint, FabricIndex fabric,
                                                                    LoadedRootCertificateCallback loadedCallback) const
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    UniquePtr<InlineBufferedRootCert> certBuffer(New<InlineBufferedRootCert>());
    UniquePtr<InlineEncodableRootCert> callbackCert(New<InlineEncodableRootCert>());
    VerifyOrReturnError(certBuffer && callbackCert, CHIP_ERROR_NO_MEMORY);

    return mCertificateTable.IterateRootCertificates(fabric, *certBuffer, [&](auto & iterator) -> CHIP_ERROR {
        while (iterator.Next(certBuffer->GetCert()))
        {
            ReturnErrorOnFailure(callbackCert->FromPersistence(fabric, certBuffer->GetCert()));
            ReturnErrorOnFailure(loadedCallback(callbackCert->inlineCertificate));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR TlsCertificateManagementCommandDelegate::RootCertsForFabric(EndpointId matterEndpoint, FabricIndex fabric,
                                                                       RootCertificateListCallback loadedCallback) const
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    uint8_t numRootCerts;
    ReturnErrorOnFailure(mCertificateTable.GetRootCertificateCount(fabric, numRootCerts));
    ScopedMemoryBuffer<InlineEncodableRootCert> rootCertificatePayloads;
    ScopedMemoryBuffer<RootCertStructType> rootCertificates;
    rootCertificatePayloads.Alloc(numRootCerts);
    rootCertificates.Alloc(numRootCerts);
    UniquePtr<InlineBufferedRootCert> certBuffer(New<InlineBufferedRootCert>());
    VerifyOrReturnError(certBuffer && !rootCertificatePayloads.IsNull() && !rootCertificates.IsNull(), CHIP_ERROR_NO_MEMORY);

    uint8_t i   = 0;
    auto result = mCertificateTable.IterateRootCertificates(fabric, *certBuffer, [&](auto & iterator) -> CHIP_ERROR {
        while (iterator.Next(certBuffer->GetCert()))
        {
            rootCertificatePayloads[i].certificate = &rootCertificates[i];
            ReturnErrorOnFailure(rootCertificatePayloads[i++].FromPersistence(fabric, certBuffer->GetCert()));
        }
        return CHIP_NO_ERROR;
    });
    ReturnErrorOnFailure(result);

    List<const RootCertStructType> rootCertificatesList(rootCertificates.Get(), numRootCerts);
    return loadedCallback(rootCertificatesList);
}

CHIP_ERROR TlsCertificateManagementCommandDelegate::FindRootCert(EndpointId matterEndpoint, FabricIndex fabric, Tls::TLSCAID id,
                                                                 LoadedRootCertificateCallback loadedCallback) const
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    UniquePtr<InlineEncodableRootCert> callbackCert(New<InlineEncodableRootCert>());
    UniquePtr<InlineBufferedRootCert> certBuffer(New<InlineBufferedRootCert>());
    ReturnErrorOnFailure(mCertificateTable.GetRootCertificateEntry(fabric, id, *certBuffer));
    ReturnErrorOnFailure(callbackCert->FromPersistence(fabric, certBuffer->GetCert()));
    return loadedCallback(callbackCert->inlineCertificate);
}

CHIP_ERROR TlsCertificateManagementCommandDelegate::LookupRootCertByFingerprint(EndpointId matterEndpoint, FabricIndex fabric,
                                                                                const ByteSpan & fingerprint,
                                                                                LoadedRootCertificateCallback loadedCallback) const
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    UniquePtr<InlineBufferedRootCert> certBuffer(New<InlineBufferedRootCert>());
    VerifyOrReturnError(certBuffer, CHIP_ERROR_NO_MEMORY);
    return mCertificateTable.IterateRootCertificates(fabric, *certBuffer, [&](auto & iterator) -> CHIP_ERROR {
        while (iterator.Next(certBuffer->GetCert()))
        {
            bool match = false;
            ReturnErrorOnFailure(FingerprintMatch(fingerprint, certBuffer->GetCert().certificate.Value(), match));
            if (match)
            {
                UniquePtr<InlineEncodableRootCert> callbackCert(New<InlineEncodableRootCert>());
                VerifyOrReturnError(callbackCert, CHIP_ERROR_NO_MEMORY);

                ReturnErrorOnFailure(callbackCert->FromPersistence(fabric, certBuffer->GetCert()));
                return loadedCallback(callbackCert->inlineCertificate);
            }
        }
        return CHIP_ERROR_NOT_FOUND;
    });
}

CHIP_ERROR TlsCertificateManagementCommandDelegate::LookupRootCert(EndpointId matterEndpoint, FabricIndex fabric,
                                                                   const ByteSpan & certificate,
                                                                   LoadedRootCertificateCallback loadedCallback) const
{
    std::array<uint8_t, Crypto::kSHA256_Hash_Length> fingerprintPayload = { 0 };
    MutableByteSpan calculatedFingerprint(fingerprintPayload);
    ReturnErrorOnFailure(Hash_SHA256(certificate.data(), certificate.size(), fingerprintPayload.data()));
    return LookupRootCertByFingerprint(matterEndpoint, fabric, calculatedFingerprint, loadedCallback);
}

Status TlsCertificateManagementCommandDelegate::RemoveRootCert(EndpointId matterEndpoint, FabricIndex fabric, Tls::TLSCAID id)
{
    VerifyOrReturnValue(matterEndpoint == EndpointId(1), Status::ConstraintError);

    auto result = mCertificateTable.RemoveRootCertificate(fabric, id);
    if (result == CHIP_ERROR_NOT_FOUND)
    {
        return Status::NotFound;
    }

    VerifyOrReturnValue(result == CHIP_NO_ERROR, Status::Failure);

    return Status::Success;
}

Status TlsCertificateManagementCommandDelegate::GenerateClientCsr(EndpointId matterEndpoint, FabricIndex fabric,
                                                                  const ClientCsrType & request,
                                                                  GeneratedCsrCallback loadedCallback) const
{
    VerifyOrReturnValue(matterEndpoint == EndpointId(1), Status::ConstraintError);

    ScopedMemoryBuffer<uint8_t> csrData;
    csrData.Alloc(kSpecMaxCertBytes);
    VerifyOrReturnValue(csrData, Status::ResourceExhausted);
    MutableByteSpan csr(csrData.Get(), kSpecMaxCertBytes);

    std::array<uint8_t, 128> nonceData;
    MutableByteSpan nonceSignature(nonceData);

    ClientCsrResponseType csrResponse;
    UniquePtr<InlineBufferedClientCert> certBuffer(New<InlineBufferedClientCert>());
    Optional<TLSCCDID> id;
    if (!request.ccdid.IsNull())
    {
        id.SetValue(request.ccdid.Value());
    }
    auto result = mCertificateTable.PrepareClientCertificate(fabric, request.nonce, certBuffer->buffer, id, csr, nonceSignature);
    ReturnValueOnFailure(result, Status::Failure);
    csrResponse.ccdid          = id.Value();
    csrResponse.csr            = csr;
    csrResponse.nonceSignature = nonceSignature;
    return loadedCallback(csrResponse);
}

Status TlsCertificateManagementCommandDelegate::ProvisionClientCert(EndpointId matterEndpoint, FabricIndex fabric,
                                                                    const ProvisionClientCertificateType & provisionReq)
{
    UniquePtr<InlineBufferedClientCert> certBuffer(New<InlineBufferedClientCert>());
    VerifyOrReturnError(certBuffer, Status::ResourceExhausted);

    TLSClientCertificateDetailStruct::DecodableType details;
    details.ccdid = provisionReq.ccdid;
    details.clientCertificate.SetValue(provisionReq.clientCertificate);
    details.intermediateCertificates.SetValue(provisionReq.intermediateCertificates);
    details.SetFabricIndex(fabric);

    auto result = mCertificateTable.UpdateClientCertificateEntry(fabric, provisionReq.ccdid, certBuffer->buffer, details);
    if (result == CHIP_ERROR_INVALID_ARGUMENT)
    {
        return Status::DynamicConstraintError;
    }
    ReturnValueOnFailure(result, Status::Failure);
    return Status::Success;
}

CHIP_ERROR TlsCertificateManagementCommandDelegate::LoadedClientCerts(EndpointId matterEndpoint, FabricIndex fabric,
                                                                      LoadedClientCertificateCallback loadedCallback) const
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    UniquePtr<InlineBufferedClientCert> certBuffer(New<InlineBufferedClientCert>());
    UniquePtr<InlineEncodableClientCert> callbackCert(New<InlineEncodableClientCert>());
    VerifyOrReturnError(certBuffer && callbackCert, CHIP_ERROR_NO_MEMORY);

    return mCertificateTable.IterateClientCertificates(fabric, *certBuffer, [&](auto & iterator) -> CHIP_ERROR {
        while (iterator.Next(certBuffer->mCertWithKey))
        {
            ReturnErrorOnFailure(callbackCert->FromPersistence(fabric, certBuffer->GetCert()));
            ReturnErrorOnFailure(loadedCallback(callbackCert->inlineCertificate));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR TlsCertificateManagementCommandDelegate::ClientCertsForFabric(EndpointId matterEndpoint, FabricIndex fabric,
                                                                         ClientCertificateListCallback loadedCallback) const
{

    VerifyOrReturnError(matterEndpoint == EndpointId(1), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    uint8_t numClientCerts;
    ReturnErrorOnFailure(mCertificateTable.GetClientCertificateCount(fabric, numClientCerts));
    UniquePtr<InlineBufferedClientCert> certBuffer(New<InlineBufferedClientCert>());
    ScopedMemoryBuffer<RefEncodableClientCert> clientCertificatePayloads;
    ScopedMemoryBuffer<ClientCertStructType> clientCertificates;
    clientCertificatePayloads.Alloc(numClientCerts);
    clientCertificates.Alloc(numClientCerts);
    VerifyOrReturnError(certBuffer && clientCertificatePayloads && clientCertificates, CHIP_ERROR_NO_MEMORY);

    uint8_t i   = 0;
    auto result = mCertificateTable.IterateClientCertificates(fabric, *certBuffer, [&](auto & iterator) -> CHIP_ERROR {
        while (iterator.Next(certBuffer->mCertWithKey))
        {
            clientCertificatePayloads[i].certificate = &clientCertificates[i];
            ReturnErrorOnFailure(clientCertificatePayloads[i++].FromPersistence(fabric, certBuffer->GetCert()));
        }
        return CHIP_NO_ERROR;
    });
    ReturnErrorOnFailure(result);

    List<const ClientCertStructType> clientCertificatesList(clientCertificates.Get(), numClientCerts);
    return loadedCallback(clientCertificatesList);
}

CHIP_ERROR TlsCertificateManagementCommandDelegate::FindClientCert(EndpointId matterEndpoint, FabricIndex fabric, TLSCCDID id,
                                                                   LoadedClientCertificateCallback loadedCallback) const
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    UniquePtr<InlineBufferedClientCert> certBuffer(New<InlineBufferedClientCert>());
    UniquePtr<InlineEncodableClientCert> callbackCert(New<InlineEncodableClientCert>());
    ReturnErrorOnFailure(mCertificateTable.GetClientCertificateEntry(fabric, id, *certBuffer));
    ReturnErrorOnFailure(callbackCert->FromPersistence(fabric, certBuffer->GetCert()));
    return loadedCallback(callbackCert->inlineCertificate);
}

CHIP_ERROR
TlsCertificateManagementCommandDelegate::LookupClientCertByFingerprint(EndpointId matterEndpoint, FabricIndex fabric,
                                                                       const ByteSpan & fingerprint,
                                                                       LoadedClientCertificateCallback loadedCallback) const
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    UniquePtr<InlineBufferedClientCert> certBuffer(New<InlineBufferedClientCert>());
    return mCertificateTable.IterateClientCertificates(fabric, *certBuffer, [&](auto & iterator) -> CHIP_ERROR {
        while (iterator.Next(certBuffer->mCertWithKey))
        {
            const auto & cert = certBuffer->GetCert();
            if (cert.clientCertificate.Value().IsNull())
            {
                continue;
            }
            bool match = false;
            ReturnErrorOnFailure(FingerprintMatch(fingerprint, cert.clientCertificate.Value().Value(), match));
            if (match)
            {
                UniquePtr<InlineEncodableClientCert> callbackCert(New<InlineEncodableClientCert>());
                VerifyOrReturnError(callbackCert, CHIP_ERROR_NO_MEMORY);

                ReturnErrorOnFailure(callbackCert->FromPersistence(fabric, cert));
                return loadedCallback(callbackCert->inlineCertificate);
            }
        }
        return CHIP_ERROR_NOT_FOUND;
    });
}

CHIP_ERROR TlsCertificateManagementCommandDelegate::LookupClientCert(EndpointId matterEndpoint, FabricIndex fabric,
                                                                     const ByteSpan & certificate,
                                                                     LoadedClientCertificateCallback loadedCallback) const
{
    std::array<uint8_t, Crypto::kSHA256_Hash_Length> fingerprintPayload = { 0 };
    MutableByteSpan calculatedFingerprint(fingerprintPayload);
    ReturnErrorOnFailure(Hash_SHA256(certificate.data(), certificate.size(), fingerprintPayload.data()));
    return LookupClientCertByFingerprint(matterEndpoint, fabric, calculatedFingerprint, loadedCallback);
}

Status TlsCertificateManagementCommandDelegate::RemoveClientCert(EndpointId matterEndpoint, FabricIndex fabric, Tls::TLSCCDID id)
{
    VerifyOrReturnValue(matterEndpoint == EndpointId(1), Status::ConstraintError);

    auto result = mCertificateTable.RemoveClientCertificate(fabric, id);
    if (result == CHIP_ERROR_NOT_FOUND)
    {
        return Status::NotFound;
    }

    VerifyOrReturnValue(result == CHIP_NO_ERROR, Status::Failure);

    return Status::Success;
}

static CertificateTableImpl gCertificateTableInstance;
TlsCertificateManagementCommandDelegate TlsCertificateManagementCommandDelegate::instance(gCertificateTableInstance);
static TlsCertificateManagementServer gTlsCertificateManagementClusterServerInstance = TlsCertificateManagementServer(
    EndpointId(1), TlsCertificateManagementCommandDelegate::GetInstance(), TlsClientManagementCommandDelegate::GetInstance(),
    gCertificateTableInstance, kMaxRootCerts, kMaxClientCerts);

void emberAfTlsCertificateManagementClusterInitCallback(EndpointId matterEndpoint)
{
    TEMPORARY_RETURN_IGNORED gCertificateTableInstance.SetEndpoint(EndpointId(1));
    TEMPORARY_RETURN_IGNORED gTlsCertificateManagementClusterServerInstance.Init();
}

void emberAfTlsCertificateManagementClusterShutdownCallback(EndpointId matterEndpoint)
{
    TEMPORARY_RETURN_IGNORED gTlsCertificateManagementClusterServerInstance.Finish();
}
