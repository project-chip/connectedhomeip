/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
 *    All rights reserved.
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

#include <algorithm>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <credentials/CHIPCert.h>
#include <lib/core/TLV.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/PersistentStorageMacros.h>
#include <lib/support/SafeInt.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/TestGroupData.h>

namespace chip {
namespace Controller {

constexpr const char kOperationalCredentialsIssuerKeypairStorage[]             = "ExampleOpCredsCAKey";
constexpr const char kOperationalCredentialsIntermediateIssuerKeypairStorage[] = "ExampleOpCredsICAKey";
constexpr const char kOperationalCredentialsRootCertificateStorage[]           = "ExampleCARootCert";
constexpr const char kOperationalCredentialsIntermediateCertificateStorage[]   = "ExampleCAIntermediateCert";

using namespace Credentials;
using namespace Crypto;
using namespace TLV;

namespace {

enum CertType : uint8_t
{
    kRcac = 0,
    kIcac = 1,
    kNoc  = 2
};

CHIP_ERROR IssueX509Cert(uint32_t now, uint32_t validity, ChipDN issuerDn, ChipDN desiredDn, CertType certType, bool maximizeSize,
                         const Crypto::P256PublicKey & subjectPublicKey, Crypto::P256Keypair & issuerKeypair,
                         MutableByteSpan & outX509Cert)
{
    constexpr size_t kDERCertFutureExtEncodingOverhead = 12;
    constexpr size_t kTLVCertFutureExtEncodingOverhead = kDERCertFutureExtEncodingOverhead + 5;
    constexpr size_t kMaxCertPaddingLength             = 200;
    constexpr size_t kTLVDesiredSize                   = kMaxCHIPCertLength;
    constexpr uint8_t sOID_Extension_SubjectAltName[]  = { 0x55, 0x1d, 0x11 };

    Platform::ScopedMemoryBuffer<uint8_t> derBuf;
    ReturnErrorCodeIf(!derBuf.Alloc(kMaxDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan derSpan{ derBuf.Get(), kMaxDERCertLength };

    int64_t serialNumber = 1;

    switch (certType)
    {
    case CertType::kRcac: {
        X509CertRequestParams rcacRequest = { serialNumber, now, now + validity, desiredDn, desiredDn };
        ReturnErrorOnFailure(NewRootX509Cert(rcacRequest, issuerKeypair, derSpan));
        break;
    }
    case CertType::kIcac: {
        X509CertRequestParams icacRequest = { serialNumber, now, now + validity, desiredDn, issuerDn };
        ReturnErrorOnFailure(NewICAX509Cert(icacRequest, subjectPublicKey, issuerKeypair, derSpan));
        break;
    }
    case CertType::kNoc: {
        X509CertRequestParams nocRequest = { serialNumber, now, now + validity, desiredDn, issuerDn };
        ReturnErrorOnFailure(NewNodeOperationalX509Cert(nocRequest, subjectPublicKey, issuerKeypair, derSpan));
        break;
    }
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (maximizeSize)
    {
        Platform::ScopedMemoryBuffer<uint8_t> paddedTlvBuf;
        ReturnErrorCodeIf(!paddedTlvBuf.Alloc(kMaxCHIPCertLength + kMaxCertPaddingLength), CHIP_ERROR_NO_MEMORY);
        MutableByteSpan paddedTlvSpan{ paddedTlvBuf.Get(), kMaxCHIPCertLength + kMaxCertPaddingLength };
        ReturnErrorOnFailure(ConvertX509CertToChipCert(derSpan, paddedTlvSpan));

        Platform::ScopedMemoryBuffer<uint8_t> paddedDerBuf;
        ReturnErrorCodeIf(!paddedDerBuf.Alloc(kMaxDERCertLength + kMaxCertPaddingLength), CHIP_ERROR_NO_MEMORY);
        MutableByteSpan paddedDerSpan{ paddedDerBuf.Get(), kMaxDERCertLength + kMaxCertPaddingLength };

        Platform::ScopedMemoryBuffer<char> fillerBuf;
        ReturnErrorCodeIf(!fillerBuf.Alloc(kMaxCertPaddingLength), CHIP_ERROR_NO_MEMORY);
        memset(fillerBuf.Get(), 'A', kMaxCertPaddingLength);

        int derPaddingLen = static_cast<int>(kMaxDERCertLength - kDERCertFutureExtEncodingOverhead - derSpan.size());
        int tlvPaddingLen = static_cast<int>(kTLVDesiredSize - kTLVCertFutureExtEncodingOverhead - paddedTlvSpan.size());
        size_t paddingLen = 0;
        if (derPaddingLen >= 1 && tlvPaddingLen >= 1)
        {
            paddingLen = std::min(static_cast<size_t>(std::min(derPaddingLen, tlvPaddingLen)), kMaxCertPaddingLength);
        }

        for (; paddingLen > 0; paddingLen--)
        {
            paddedDerSpan = MutableByteSpan{ paddedDerBuf.Get(), kMaxDERCertLength + kMaxCertPaddingLength };
            paddedTlvSpan = MutableByteSpan{ paddedTlvBuf.Get(), kMaxCHIPCertLength + kMaxCertPaddingLength };

            Optional<FutureExtension> futureExt;
            FutureExtension ext = { ByteSpan(sOID_Extension_SubjectAltName),
                                    ByteSpan(reinterpret_cast<uint8_t *>(fillerBuf.Get()), paddingLen) };
            futureExt.SetValue(ext);

            switch (certType)
            {
            case CertType::kRcac: {
                X509CertRequestParams rcacRequest = { serialNumber, now, now + validity, desiredDn, desiredDn, futureExt };
                ReturnErrorOnFailure(NewRootX509Cert(rcacRequest, issuerKeypair, paddedDerSpan));
                break;
            }
            case CertType::kIcac: {
                X509CertRequestParams icacRequest = { serialNumber, now, now + validity, desiredDn, issuerDn, futureExt };
                ReturnErrorOnFailure(NewICAX509Cert(icacRequest, subjectPublicKey, issuerKeypair, paddedDerSpan));
                break;
            }
            case CertType::kNoc: {
                X509CertRequestParams nocRequest = { serialNumber, now, now + validity, desiredDn, issuerDn, futureExt };
                ReturnErrorOnFailure(NewNodeOperationalX509Cert(nocRequest, subjectPublicKey, issuerKeypair, paddedDerSpan));
                break;
            }
            default:
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            ReturnErrorOnFailure(ConvertX509CertToChipCert(paddedDerSpan, paddedTlvSpan));

            if (paddedDerSpan.size() <= kMaxDERCertLength && paddedTlvSpan.size() <= kMaxCHIPCertLength)
            {
                ChipLogProgress(Controller, "Generated maximized certificate with %u DER bytes, %u TLV bytes",
                                static_cast<unsigned>(paddedDerSpan.size()), static_cast<unsigned>(paddedTlvSpan.size()));
                return CopySpanToMutableSpan(paddedDerSpan, outX509Cert);
            }
        }
    }

    return CopySpanToMutableSpan(derSpan, outX509Cert);
}

} // namespace

CHIP_ERROR ExampleOperationalCredentialsIssuer::Initialize(PersistentStorageDelegate & storage)
{
    using namespace ASN1;
    ASN1UniversalTime effectiveTime;
    CHIP_ERROR err;

    // Initializing the default start validity to start of 2021. The default validity duration is 10 years.
    CHIP_ZERO_AT(effectiveTime);
    effectiveTime.Year  = 2021;
    effectiveTime.Month = 1;
    effectiveTime.Day   = 1;
    ReturnErrorOnFailure(ASN1ToChipEpochTime(effectiveTime, mNow));

    Crypto::P256SerializedKeypair serializedKey;
    {
        // Scope for keySize, because we use it as an in/out param.
        uint16_t keySize = static_cast<uint16_t>(serializedKey.Capacity());

        PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsIssuerKeypairStorage, key,
                          err = storage.SyncGetKeyValue(key, serializedKey.Bytes(), keySize));
        serializedKey.SetLength(keySize);
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Controller, "Couldn't get %s from storage: %s", kOperationalCredentialsIssuerKeypairStorage, ErrorStr(err));
        // Storage doesn't have an existing keypair. Let's create one and add it to the storage.
        ReturnErrorOnFailure(mIssuer.Initialize(Crypto::ECPKeyTarget::ECDSA));
        ReturnErrorOnFailure(mIssuer.Serialize(serializedKey));

        PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsIssuerKeypairStorage, key,
                          ReturnErrorOnFailure(
                              storage.SyncSetKeyValue(key, serializedKey.Bytes(), static_cast<uint16_t>(serializedKey.Length()))));
    }
    else
    {
        // Use the keypair from the storage
        ReturnErrorOnFailure(mIssuer.Deserialize(serializedKey));
    }

    {
        // Scope for keySize, because we use it as an in/out param.
        uint16_t keySize = static_cast<uint16_t>(serializedKey.Capacity());

        PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsIntermediateIssuerKeypairStorage, key,
                          err = storage.SyncGetKeyValue(key, serializedKey.Bytes(), keySize));
        serializedKey.SetLength(keySize);
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Controller, "Couldn't get %s from storage: %s", kOperationalCredentialsIntermediateIssuerKeypairStorage,
                        ErrorStr(err));
        // Storage doesn't have an existing keypair. Let's create one and add it to the storage.
        ReturnErrorOnFailure(mIntermediateIssuer.Initialize(Crypto::ECPKeyTarget::ECDSA));
        ReturnErrorOnFailure(mIntermediateIssuer.Serialize(serializedKey));

        PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsIntermediateIssuerKeypairStorage, key,
                          ReturnErrorOnFailure(
                              storage.SyncSetKeyValue(key, serializedKey.Bytes(), static_cast<uint16_t>(serializedKey.Length()))));
    }
    else
    {
        // Use the keypair from the storage
        ReturnErrorOnFailure(mIntermediateIssuer.Deserialize(serializedKey));
    }

    mStorage     = &storage;
    mInitialized = true;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ExampleOperationalCredentialsIssuer::GenerateNOCChainAfterValidation(NodeId nodeId, FabricId fabricId,
                                                                                const CATValues & cats,
                                                                                const Crypto::P256PublicKey & pubkey,
                                                                                MutableByteSpan & rcac, MutableByteSpan & icac,
                                                                                MutableByteSpan & noc)
{
    ChipDN rcac_dn;
    CHIP_ERROR err      = CHIP_NO_ERROR;
    uint16_t rcacBufLen = static_cast<uint16_t>(std::min(rcac.size(), static_cast<size_t>(UINT16_MAX)));
    PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsRootCertificateStorage, key,
                      err = mStorage->SyncGetKeyValue(key, rcac.data(), rcacBufLen));
    // Always regenerate RCAC on maximally sized certs. The keys remain the same, so everything is fine.
    if (mUseMaximallySizedCerts)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    if (err == CHIP_NO_ERROR)
    {
        uint64_t rcacId;
        // Found root certificate in the storage.
        rcac.reduce_size(rcacBufLen);
        ReturnErrorOnFailure(ExtractSubjectDNFromX509Cert(rcac, rcac_dn));
        ReturnErrorOnFailure(rcac_dn.GetCertChipId(rcacId));
        VerifyOrReturnError(rcacId == mIssuerId, CHIP_ERROR_INTERNAL);
    }
    // If root certificate not found in the storage, generate new root certificate.
    else
    {
        ReturnErrorOnFailure(rcac_dn.AddAttribute_MatterRCACId(mIssuerId));

        ChipLogProgress(Controller, "Generating RCAC");
        ReturnErrorOnFailure(IssueX509Cert(mNow, mValidity, rcac_dn, rcac_dn, CertType::kRcac, mUseMaximallySizedCerts,
                                           mIssuer.Pubkey(), mIssuer, rcac));
        VerifyOrReturnError(CanCastTo<uint16_t>(rcac.size()), CHIP_ERROR_INTERNAL);

        // Re-extract DN based on final generated cert
        rcac_dn = ChipDN{};
        ReturnErrorOnFailure(ExtractSubjectDNFromX509Cert(rcac, rcac_dn));

        PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsRootCertificateStorage, key,
                          ReturnErrorOnFailure(mStorage->SyncSetKeyValue(key, rcac.data(), static_cast<uint16_t>(rcac.size()))));
    }

    ChipDN icac_dn;
    uint16_t icacBufLen = static_cast<uint16_t>(std::min(icac.size(), static_cast<size_t>(UINT16_MAX)));
    PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsIntermediateCertificateStorage, key,
                      err = mStorage->SyncGetKeyValue(key, icac.data(), icacBufLen));
    // Always regenerate ICAC on maximally sized certs. The keys remain the same, so everything is fine.
    if (mUseMaximallySizedCerts)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    if (err == CHIP_NO_ERROR)
    {
        uint64_t icacId;
        // Found intermediate certificate in the storage.
        icac.reduce_size(icacBufLen);
        ReturnErrorOnFailure(ExtractSubjectDNFromX509Cert(icac, icac_dn));
        ReturnErrorOnFailure(icac_dn.GetCertChipId(icacId));
        VerifyOrReturnError(icacId == mIntermediateIssuerId, CHIP_ERROR_INTERNAL);
    }
    // If intermediate certificate not found in the storage, generate new intermediate certificate.
    else
    {
        ReturnErrorOnFailure(icac_dn.AddAttribute_MatterICACId(mIntermediateIssuerId));

        ChipLogProgress(Controller, "Generating ICAC");
        ReturnErrorOnFailure(IssueX509Cert(mNow, mValidity, rcac_dn, icac_dn, CertType::kIcac, mUseMaximallySizedCerts,
                                           mIntermediateIssuer.Pubkey(), mIssuer, icac));
        VerifyOrReturnError(CanCastTo<uint16_t>(icac.size()), CHIP_ERROR_INTERNAL);

        // Re-extract DN based on final generated cert
        icac_dn = ChipDN{};
        ReturnErrorOnFailure(ExtractSubjectDNFromX509Cert(icac, icac_dn));

        PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsIntermediateCertificateStorage, key,
                          ReturnErrorOnFailure(mStorage->SyncSetKeyValue(key, icac.data(), static_cast<uint16_t>(icac.size()))));
    }

    ChipDN noc_dn;
    ReturnErrorOnFailure(noc_dn.AddAttribute_MatterFabricId(fabricId));
    ReturnErrorOnFailure(noc_dn.AddAttribute_MatterNodeId(nodeId));
    ReturnErrorOnFailure(noc_dn.AddCATs(cats));

    ChipLogProgress(Controller, "Generating NOC");
    return IssueX509Cert(mNow, mValidity, icac_dn, noc_dn, CertType::kNoc, mUseMaximallySizedCerts, pubkey, mIntermediateIssuer,
                         noc);
}

CHIP_ERROR ExampleOperationalCredentialsIssuer::GenerateNOCChain(const ByteSpan & csrElements, const ByteSpan & csrNonce,
                                                                 const ByteSpan & attestationSignature,
                                                                 const ByteSpan & attestationChallenge, const ByteSpan & DAC,
                                                                 const ByteSpan & PAI,
                                                                 Callback::Callback<OnNOCChainGeneration> * onCompletion)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_WELL_UNINITIALIZED);
    // At this point, Credential issuer may wish to validate the CSR information
    (void) attestationChallenge;
    (void) csrNonce;

    NodeId assignedId;
    if (mNodeIdRequested)
    {
        assignedId       = mNextRequestedNodeId;
        mNodeIdRequested = false;
    }
    else
    {
        assignedId = mNextAvailableNodeId++;
    }

    ChipLogProgress(Controller, "Verifying Certificate Signing Request");
    TLVReader reader;
    reader.Init(csrElements);

    if (reader.GetType() == kTLVType_NotSpecified)
    {
        ReturnErrorOnFailure(reader.Next());
    }

    VerifyOrReturnError(reader.GetType() == kTLVType_Structure, CHIP_ERROR_WRONG_TLV_TYPE);
    VerifyOrReturnError(reader.GetTag() == AnonymousTag(), CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);

    TLVType containerType;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    ReturnErrorOnFailure(reader.Next(kTLVType_ByteString, TLV::ContextTag(1)));

    ByteSpan csr(reader.GetReadPoint(), reader.GetLength());
    reader.ExitContainer(containerType);

    P256PublicKey pubkey;
    ReturnErrorOnFailure(VerifyCertificateSigningRequest(csr.data(), csr.size(), pubkey));

    chip::Platform::ScopedMemoryBuffer<uint8_t> noc;
    ReturnErrorCodeIf(!noc.Alloc(kMaxDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan nocSpan(noc.Get(), kMaxDERCertLength);

    chip::Platform::ScopedMemoryBuffer<uint8_t> icac;
    ReturnErrorCodeIf(!icac.Alloc(kMaxDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan icacSpan(icac.Get(), kMaxDERCertLength);

    chip::Platform::ScopedMemoryBuffer<uint8_t> rcac;
    ReturnErrorCodeIf(!rcac.Alloc(kMaxDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan rcacSpan(rcac.Get(), kMaxDERCertLength);

    ReturnErrorOnFailure(
        GenerateNOCChainAfterValidation(assignedId, mNextFabricId, mNextCATs, pubkey, rcacSpan, icacSpan, nocSpan));

    // TODO(#13825): Should always generate some IPK. Using a temporary fixed value until APIs are plumbed in to set it end-to-end
    // TODO: Force callers to set IPK if used before GenerateNOCChain will succeed.
    ByteSpan defaultIpkSpan = chip::GroupTesting::DefaultIpkValue::GetDefaultIpk();

    // The below static assert validates a key assumption in types used (needed for public API conformance)
    static_assert(CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES == kAES_CCM128_Key_Length, "IPK span sizing must match");

    // Prepare IPK to be sent back. A more fully-fledged operational credentials delegate
    // would obtain a suitable key per fabric.
    uint8_t ipkValue[CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];
    Crypto::IdentityProtectionKeySpan ipkSpan(ipkValue);

    ReturnErrorCodeIf(defaultIpkSpan.size() != sizeof(ipkValue), CHIP_ERROR_INTERNAL);
    memcpy(&ipkValue[0], defaultIpkSpan.data(), defaultIpkSpan.size());

    // Callback onto commissioner.
    ChipLogProgress(Controller, "Providing certificate chain to the commissioner");
    onCompletion->mCall(onCompletion->mContext, CHIP_NO_ERROR, nocSpan, icacSpan, rcacSpan, MakeOptional(ipkSpan),
                        Optional<NodeId>());
    return CHIP_NO_ERROR;
}

CHIP_ERROR ExampleOperationalCredentialsIssuer::GetRandomOperationalNodeId(NodeId * aNodeId)
{
    for (int i = 0; i < 10; ++i)
    {
        CHIP_ERROR err = DRBG_get_bytes(reinterpret_cast<uint8_t *>(aNodeId), sizeof(*aNodeId));
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }

        if (IsOperationalNodeId(*aNodeId))
        {
            return CHIP_NO_ERROR;
        }
    }

    // Terrible, universe-ending luck (chances are 1 in 2^280 or so here, if our
    // DRBG is good).
    return CHIP_ERROR_INTERNAL;
}

} // namespace Controller
} // namespace chip
