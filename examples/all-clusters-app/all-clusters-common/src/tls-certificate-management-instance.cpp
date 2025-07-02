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

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Tls;
using namespace chip::app::Clusters::TlsCertificateManagement;
using namespace Protocols::InteractionModel;
using namespace chip::Crypto;
using namespace chip::Platform;

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

static constexpr uint8_t kMaxRootCerts   = 254;
static constexpr uint8_t kMaxClientCerts = 254;

ClusterStatusCode TlsCertificateManagementCommandDelegate::ProvisionRootCert(EndpointId matterEndpoint, FabricIndex fabric,
                                                                             const ProvisionRootCertificateType & provisionReq,
                                                                             Tls::TLSCAID & outCaid)
{
    auto result = mCertificateTable.UpsertRootCertificateEntry(matterEndpoint, fabric, outCaid, provisionReq);

    VerifyOrReturnValue(result == CHIP_NO_ERROR, ClusterStatusCode(Status::Failure));
    return ClusterStatusCode(Status::Success);
}

CHIP_ERROR TlsCertificateManagementCommandDelegate::LoadedRootCerts(EndpointId matterEndpoint, FabricIndex fabric,
                                                                    LoadedRootCertificateCallback loadedCallback) const
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    AutoDelete<InlineBufferedRootCert> certBuffer(Platform::New<InlineBufferedRootCert>());
    VerifyOrReturnError(!certBuffer.IsNull(), CHIP_ERROR_NO_MEMORY);

    return mCertificateTable.IterateRootCertificates(fabric, *certBuffer, [&](auto & iterator) -> CHIP_ERROR {
        while (iterator.Next(certBuffer->mCert))
        {
            ReturnErrorOnFailure(loadedCallback(certBuffer->mCert));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR TlsCertificateManagementCommandDelegate::RootCertsForFabric(EndpointId matterEndpoint, FabricIndex fabric,
                                                                       RootCertificateListCallback loadedCallback) const
{}

CHIP_ERROR TlsCertificateManagementCommandDelegate::FindRootCert(EndpointId matterEndpoint, FabricIndex fabric, CAID id,
                                                                 LoadedRootCertificateCallback loadedCallback) const
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    AutoDelete<InlineBufferedRootCert> certBuffer(Platform::New<InlineBufferedRootCert>());
    ReturnErrorOnFailure(mCertificateTable.GetRootCertificateEntry(fabric, id, *certBuffer));
    return loadedCallback(certBuffer->mCert);
}

CHIP_ERROR TlsCertificateManagementCommandDelegate::LookupRootCert(EndpointId matterEndpoint, FabricIndex fabric,
                                                                   const ByteSpan & fingerprint,
                                                                   LoadedRootCertificateCallback loadedCallback) const
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    AutoDelete<InlineBufferedRootCert> certBuffer(Platform::New<InlineBufferedRootCert>());
    return mCertificateTable.IterateRootCertificates(fabric, *certBuffer, [&](auto & iterator) -> CHIP_ERROR {
        while (iterator.Next(certBuffer->mCert))
        {
            uint8_t fingerprintPayload[chip::Crypto::kSHA1_Hash_Length] = { 0 };
            MutableByteSpan calculatedFingerprint(fingerprintPayload);
            auto & cert = certBuffer->mCert.Certificate;
            Hash_SHA1(cert.Data(), cert.size(), calculatedFingerprint);
            if (data_equal(fingerprint, calculatedFingerprint))
            {
                return loadedCallback(certBuffer->mCert);
            }
        }
        return CHIP_ERROR_NOT_FOUND;
    });
}

CHIP_ERROR TlsCertificateManagementCommandDelegate::RemoveRootCert(EndpointId matterEndpoint, FabricIndex fabric, Tls::TLSCAID id)
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    return mCertificateTable.RemoveRootCertificate(fabric, id);
}

Status TlsCertificateManagementCommandDelegate::GenerateClientCsr(EndpointId matterEndpoint, FabricIndex fabric,
                                                                  const ClientCsrType & request,
                                                                  GeneratedCsrCallback loadedCallback) const
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    AutoDelete<std::array<uint8_t, 3000>> csrData(Platform::New<std::array<uint8_t, 3000>>());
    MutableByteSpan csr(*csrData);
    std::array<uint8_t, 128> nonceData;
    MutableByteSpan nonceSignature(nonceData);

    ClientCsrResponseType csResponse;
    mCertificateTable.PrepareClientCertificate(matterEndpoint, fabric, csrResponse.ccdid, csr, nonceSignature);
    csrResponse.csr   = csr;
    csrResponse.nonce = nonceSignature;
    return loadedCallback(csrResponse);
}

ClusterStatusCode TlsCertificateManagementCommandDelegate::ProvisionClientCert(EndpointId matterEndpoint, FabricIndex fabric,
                                                                               const ProvisionClientCertificateType & provisionReq)
{
    auto result = mCertificateTable.UpdateClientCertificateEntry(fabric, provisionReq);
    VerifyOrReturnValue(result == CHIP_NO_ERROR, ClusterStatusCode(Status::Failure));
    return ClusterStatusCode(Status::Success);
}

CHIP_ERROR TlsCertificateManagementCommandDelegate::LoadedClientCerts(EndpointId matterEndpoint, FabricIndex fabric,
                                                                      LoadedClientCertificateCallback loadedCallback) const
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    AutoDelete<InlineBufferedClientCert> certBuffer(Platform::New<InlineBufferedClientCert>());
    VerifyOrReturnError(!certBuffer.IsNull(), CHIP_ERROR_NO_MEMORY);

    return mCertificateTable.IterateClientCertificates(fabric, *certBuffer, [&](auto & iterator) -> CHIP_ERROR {
        while (iterator.Next(certBuffer->mCert))
        {
            ReturnErrorOnFailure(loadedCallback(certBuffer->mCert));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR TlsCertificateManagementCommandDelegate::ClientCertsForFabric(EndpointId matterEndpoint, FabricIndex fabric,
                                                                         ClientCertificateListCallback loadedCallback) const
{}

CHIP_ERROR TlsCertificateManagementCommandDelegate::FindClientCert(EndpointId matterEndpoint, FabricIndex fabric, TLSCCDID id,
                                                                   LoadedClientCertificateCallback loadedCallback) const
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    AutoDelete<InlineBufferedRootCert> certBuffer(Platform::New<InlineBufferedClientCert>());
    ReturnErrorOnFailure(mCertificateTable.GetClientCertificateEntry(fabric, id, *certBuffer));
    return loadedCallback(certBuffer->mCert);
}

CHIP_ERROR TlsCertificateManagementCommandDelegate::LookupClientCert(EndpointId matterEndpoint, FabricIndex fabric,
                                                                     const ByteSpan & fingerprint,
                                                                     LoadedClientCertificateCallback loadedCallback) const
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    AutoDelete<InlineBufferedRootCert> certBuffer(Platform::New<InlineBufferedClientCert>());
    return mCertificateTable.IterateClientCertificates(fabric, *certBuffer, [&](auto & iterator) -> CHIP_ERROR {
        while (iterator.Next(certBuffer->mCert))
        {
            uint8_t fingerprintPayload[chip::Crypto::kSHA1_Hash_Length] = { 0 };
            MutableByteSpan calculatedFingerprint(fingerprintPayload);
            auto & cert = certBuffer->mCert.Certificate;
            Hash_SHA1(cert.Data(), cert.size(), calculatedFingerprint);
            if (data_equal(fingerprint, calculatedFingerprint))
            {
                return loadedCallback(certBuffer->mCert);
            }
        }
        return CHIP_ERROR_NOT_FOUND;
    });
}

CHIP_ERROR TlsCertificateManagementCommandDelegate::RemoveClientCert(EndpointId matterEndpoint, FabricIndex fabric, Tls::TLSCAID id)
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    return mCertificateTable.RemoveClientCertificate(fabric, id);
}

static CertificateTableImpl gCertificateTableInstance;
TlsCertificateManagementCommandDelegate TlsCertificateManagementCommandDelegate::instance(gCertificateTableInstance);
static TlsCertificateManagementServer gTlsCertificateManagementClusterServerInstance =
    TlsCertificateManagementServer(EndpointId(1), TlsCertificateManagementCommandDelegate::getInstance(), gCertificateTableInstance,
                                   kMaxRootCerts, kMaxClientCerts);

void emberAfTlsCertificateManagementClusterInitCallback(EndpointId matterEndpoint)
{
    gCertificateTableInstance.SetEndpoint(EndpointId(1));
    gTlsCertificateManagementClusterServerInstance.Init();
}

void emberAfTlsCertificateManagementClusterShutdownCallback(EndpointId matterEndpoint)
{
    gTlsCertificateManagementClusterServerInstance.Finish();
}
