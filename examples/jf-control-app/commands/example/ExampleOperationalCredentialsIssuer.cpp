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
#include <credentials/CHIPCert.h>
#include <lib/core/TLV.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/PersistentStorageMacros.h>
#include <lib/support/SafeInt.h>
#include <lib/support/ScopedMemoryBuffer.h>
#include <lib/support/TestGroupData.h>

#include "commands/example/ExampleOperationalCredentialsIssuer.h"

namespace chip {
namespace CustomCredentialsIssuer {

constexpr char kOperationalCredentialsIssuerKeypairStorage[]             = "ExampleOpCredsCAKey";
constexpr char kOperationalCredentialsIntermediateIssuerKeypairStorage[] = "ExampleOpCredsICAKey";
constexpr char kOperationalCredentialsRootCertificateStorage[]           = "ExampleCARootCert";
constexpr char kOperationalCredentialsIntermediateCertificateStorage[]   = "ExampleCAIntermediateCert";

constexpr char kOperationalCredentialsAnchorIntermediateIssuerKeypairStorage[] = "ExampleOpCredsAnchorICAKey";
constexpr char kOperationalCredentialsAnchorIntermediateCertificateStorage[]   = "ExampleCAAnchorIntermediateCert";

constexpr char kJFMaterialsICACStorage[] = "ExampleJFMaterialsICAC";
constexpr char kJFMaterialsRCACStorage[] = "ExampleJFMaterialsRCAC";
constexpr char kJFMaterialsIPKStorage[]  = "ExampleJFMaterialsIPK";

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
    VerifyOrReturnError(derBuf.Alloc(kMaxDERCertLength), CHIP_ERROR_NO_MEMORY);
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
        VerifyOrReturnError(paddedTlvBuf.Alloc(kMaxCHIPCertLength + kMaxCertPaddingLength), CHIP_ERROR_NO_MEMORY);
        MutableByteSpan paddedTlvSpan{ paddedTlvBuf.Get(), kMaxCHIPCertLength + kMaxCertPaddingLength };
        ReturnErrorOnFailure(ConvertX509CertToChipCert(derSpan, paddedTlvSpan));

        Platform::ScopedMemoryBuffer<uint8_t> paddedDerBuf;
        VerifyOrReturnError(paddedDerBuf.Alloc(kMaxDERCertLength + kMaxCertPaddingLength), CHIP_ERROR_NO_MEMORY);
        MutableByteSpan paddedDerSpan{ paddedDerBuf.Get(), kMaxDERCertLength + kMaxCertPaddingLength };

        Platform::ScopedMemoryBuffer<char> fillerBuf;
        VerifyOrReturnError(fillerBuf.Alloc(kMaxCertPaddingLength), CHIP_ERROR_NO_MEMORY);
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
        err = (err == CHIP_NO_ERROR) ? serializedKey.SetLength(keySize) : err;
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Controller, "Couldn't get %s from storage: %s", kOperationalCredentialsIssuerKeypairStorage, ErrorStr(err));
        if (mKeypairsInitialized)
        {
            // A keypair is already loaded in memory from a previous Initialize() under a different
            // per-identity storage namespace. Reuse it (and write it through to this storage) rather
            // than regenerating a fresh key, so the issuer keys stay stable across identities. This is
            // required post-JCM: the cross-signed ICAC embeds the anchor intermediate public key and the
            // Fabric-A controller NOC must be signed with the matching private key.
            ChipLogProgress(Controller, "Reusing in-memory %s and persisting to current storage",
                            kOperationalCredentialsIssuerKeypairStorage);
            ReturnErrorOnFailure(mIssuer.Serialize(serializedKey));
            PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsIssuerKeypairStorage, key,
                              ReturnErrorOnFailure(storage.SyncSetKeyValue(key, serializedKey.Bytes(),
                                                                           static_cast<uint16_t>(serializedKey.Length()))));
        }
        else
        {
            // Storage doesn't have an existing keypair. Let's create one and add it to the storage.
            ReturnErrorOnFailure(mIssuer.Initialize(Crypto::ECPKeyTarget::ECDSA));
            ReturnErrorOnFailure(mIssuer.Serialize(serializedKey));

            PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsIssuerKeypairStorage, key,
                              ReturnErrorOnFailure(storage.SyncSetKeyValue(key, serializedKey.Bytes(),
                                                                           static_cast<uint16_t>(serializedKey.Length()))));
        }
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
        err = (err == CHIP_NO_ERROR) ? serializedKey.SetLength(keySize) : err;
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Controller, "Couldn't get %s from storage: %s", kOperationalCredentialsIntermediateIssuerKeypairStorage,
                        ErrorStr(err));
        if (mKeypairsInitialized)
        {
            // Reuse the in-memory intermediate keypair (loaded under a previous storage namespace) and
            // persist it to this storage, rather than regenerating a fresh key. See the comment on the
            // mIssuer block above for the rationale (stable issuer keys across identities post-JCM).
            ChipLogProgress(Controller, "Reusing in-memory %s and persisting to current storage",
                            kOperationalCredentialsIntermediateIssuerKeypairStorage);
            ReturnErrorOnFailure(mIntermediateIssuer.Serialize(serializedKey));
            PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsIntermediateIssuerKeypairStorage, key,
                              ReturnErrorOnFailure(storage.SyncSetKeyValue(key, serializedKey.Bytes(),
                                                                           static_cast<uint16_t>(serializedKey.Length()))));
        }
        else
        {
            // Storage doesn't have an existing keypair. Let's create one and add it to the storage.
            ReturnErrorOnFailure(mIntermediateIssuer.Initialize(Crypto::ECPKeyTarget::ECDSA));
            ReturnErrorOnFailure(mIntermediateIssuer.Serialize(serializedKey));

            PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsIntermediateIssuerKeypairStorage, key,
                              ReturnErrorOnFailure(storage.SyncSetKeyValue(key, serializedKey.Bytes(),
                                                                           static_cast<uint16_t>(serializedKey.Length()))));
        }
    }
    else
    {
        // Use the keypair from the storage
        ReturnErrorOnFailure(mIntermediateIssuer.Deserialize(serializedKey));
    }

    {
        // Scope for keySize, because we use it as an in/out param.
        uint16_t keySize = static_cast<uint16_t>(serializedKey.Capacity());

        PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsAnchorIntermediateIssuerKeypairStorage, key,
                          err = storage.SyncGetKeyValue(key, serializedKey.Bytes(), keySize));
        err = (err == CHIP_NO_ERROR) ? serializedKey.SetLength(keySize) : err;
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Controller, "Couldn't get %s from storage: %s",
                        kOperationalCredentialsAnchorIntermediateIssuerKeypairStorage, ErrorStr(err));
        if (mKeypairsInitialized)
        {
            // Reuse the in-memory anchor intermediate keypair (loaded under a previous storage
            // namespace) and persist it to this storage, rather than regenerating a fresh key. This is
            // the key fix: the cross-signed JF-materials ICAC embeds this public key, so the Fabric-A
            // controller NOC issued for this identity must be signed with the SAME private key, or NOC
            // chain validation on the migrated node fails (CA cert not found / AKID mismatch).
            ChipLogProgress(Controller, "Reusing in-memory %s and persisting to current storage",
                            kOperationalCredentialsAnchorIntermediateIssuerKeypairStorage);
            ReturnErrorOnFailure(mAnchorIntermediateIssuer.Serialize(serializedKey));
            PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsAnchorIntermediateIssuerKeypairStorage, key,
                              ReturnErrorOnFailure(storage.SyncSetKeyValue(key, serializedKey.Bytes(),
                                                                           static_cast<uint16_t>(serializedKey.Length()))));
        }
        else
        {
            // Storage doesn't have an existing keypair. Let's create one and add it to the storage.
            ReturnErrorOnFailure(mAnchorIntermediateIssuer.Initialize(Crypto::ECPKeyTarget::ECDSA));
            ReturnErrorOnFailure(mAnchorIntermediateIssuer.Serialize(serializedKey));

            PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsAnchorIntermediateIssuerKeypairStorage, key,
                              ReturnErrorOnFailure(storage.SyncSetKeyValue(key, serializedKey.Bytes(),
                                                                           static_cast<uint16_t>(serializedKey.Length()))));
        }
    }
    else
    {
        // Use the keypair from the storage
        ReturnErrorOnFailure(mAnchorIntermediateIssuer.Deserialize(serializedKey));
    }

    // All three issuer keypairs are now loaded (either from storage, reused from memory, or freshly
    // generated). Mark them initialized so a later Initialize() under a different per-identity storage
    // namespace reuses these same keys instead of regenerating.
    mKeypairsInitialized = true;

    mStorage     = &storage;
    mInitialized = true;

    // Restore persisted JF materials if present.
    {
        uint16_t icacLen = static_cast<uint16_t>(sizeof(mJFMaterialsICAC));
        uint16_t rcacLen = static_cast<uint16_t>(sizeof(mJFMaterialsRCAC));
        uint16_t ipkLen  = static_cast<uint16_t>(sizeof(mJFMaterialsIPK));

        CHIP_ERROR icacErr = storage.SyncGetKeyValue(kJFMaterialsICACStorage, mJFMaterialsICAC, icacLen);
        CHIP_ERROR rcacErr = storage.SyncGetKeyValue(kJFMaterialsRCACStorage, mJFMaterialsRCAC, rcacLen);
        CHIP_ERROR ipkErr  = storage.SyncGetKeyValue(kJFMaterialsIPKStorage, mJFMaterialsIPK, ipkLen);

        if (icacErr == CHIP_NO_ERROR && rcacErr == CHIP_NO_ERROR && ipkErr == CHIP_NO_ERROR)
        {
            mJFMaterialsICACLen = icacLen;
            mJFMaterialsRCACLen = rcacLen;
            mJFMaterialsIPKLen  = ipkLen;
            mHasJFMaterials     = true;
            ChipLogProgress(Controller, "Initialize: restored JF materials from storage (ICAC=%u, RCAC=%u, IPK=%u bytes)",
                            static_cast<unsigned>(icacLen), static_cast<unsigned>(rcacLen), static_cast<unsigned>(ipkLen));
        }
        else
        {
            ChipLogProgress(Controller, "Initialize: no persisted JF materials found");
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExampleOperationalCredentialsIssuer::GenerateNOCChainAfterValidation(NodeId nodeId, FabricId fabricId,
                                                                                const CATValues & cats,
                                                                                const Crypto::P256PublicKey & pubkey,
                                                                                MutableByteSpan & rcac, MutableByteSpan & icac,
                                                                                MutableByteSpan & noc)
{
    // After JCM / node migration, JFC-B must operate on Fabric A. When JF materials received from JFA-A are
    // present, issue the controller NOC chain directly on Fabric A instead of minting a fresh local chain:
    // return the Fabric-A RCAC and the cross-signed anchor ICAC as-is, derive the real Fabric-A fabric id from
    // the anchor ICAC, and sign a NOC that carries the caller-provided CATs (Admin CAT) under that fabric.
    if (mHasJFMaterials)
    {
        ChipLogProgress(Controller, "GenerateNOCChainAfterValidation: using JF materials from JFA-A (ICAC=%u, RCAC=%u bytes)",
                        static_cast<unsigned>(mJFMaterialsICACLen), static_cast<unsigned>(mJFMaterialsRCACLen));

        {
            ByteSpan rcacTlv(mJFMaterialsRCAC, mJFMaterialsRCACLen);
            ReturnErrorOnFailure(ConvertChipCertToX509Cert(rcacTlv, rcac));
        }
        {
            ByteSpan icacTlv(mJFMaterialsICAC, mJFMaterialsICACLen);
            ReturnErrorOnFailure(ConvertChipCertToX509Cert(icacTlv, icac));
        }

        ChipDN anchor_icac_dn;
        {
            ByteSpan icacTlv(mJFMaterialsICAC, mJFMaterialsICACLen);
            ReturnErrorOnFailure(ExtractSubjectDNFromChipCert(icacTlv, anchor_icac_dn));
        }

        FabricId anchorFabricId;
        ReturnErrorOnFailure(anchor_icac_dn.GetCertFabricId(anchorFabricId));

        ChipDN jf_noc_dn;
        ReturnErrorOnFailure(jf_noc_dn.AddAttribute_MatterFabricId(anchorFabricId));
        ReturnErrorOnFailure(jf_noc_dn.AddAttribute_MatterNodeId(nodeId));
        ReturnErrorOnFailure(jf_noc_dn.AddCATs(cats));

        ChipLogProgress(Controller,
                        "GenerateNOCChainAfterValidation: generating Fabric-A controller NOC (fabricId=0x" ChipLogFormatX64 ")",
                        ChipLogValueX64(anchorFabricId));
        return IssueX509Cert(mNow, mValidity, anchor_icac_dn, jf_noc_dn, CertType::kNoc, mUseMaximallySizedCerts, pubkey,
                             mAnchorIntermediateIssuer, noc);
    }

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
    uint16_t icacBufLen  = static_cast<uint16_t>(std::min(icac.size(), static_cast<size_t>(UINT16_MAX)));
    bool isForAnchorIcac = cats.ContainsIdentifier(kAnchorCATIdentifier);

    if (!isForAnchorIcac)
    {
        PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsIntermediateCertificateStorage, key,
                          err = mStorage->SyncGetKeyValue(key, icac.data(), icacBufLen));
    }
    else
    {
        PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsAnchorIntermediateCertificateStorage, key,
                          err = mStorage->SyncGetKeyValue(key, icac.data(), icacBufLen));
    }

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
        VerifyOrReturnError(icacId == (isForAnchorIcac ? mAnchorIntermediateIssuerId : mIntermediateIssuerId), CHIP_ERROR_INTERNAL);
    }
    // If intermediate certificate not found in the storage, generate new intermediate certificate.
    else
    {
        ChipLogProgress(Controller, "Generating ICAC");
        if (!isForAnchorIcac)
        {
            ReturnErrorOnFailure(icac_dn.AddAttribute_MatterICACId(mIntermediateIssuerId));
            ReturnErrorOnFailure(IssueX509Cert(mNow, mValidity, rcac_dn, icac_dn, CertType::kIcac, mUseMaximallySizedCerts,
                                               mIntermediateIssuer.Pubkey(), mIssuer, icac));
        }
        else
        {
            ReturnErrorOnFailure(icac_dn.AddAttribute_MatterICACId(mAnchorIntermediateIssuerId));
            ReturnErrorOnFailure(icac_dn.AddAttribute_OrganizationalUnitName("jf-anchor-icac"_span, false));
            ReturnErrorOnFailure(IssueX509Cert(mNow, mValidity, rcac_dn, icac_dn, CertType::kIcac, mUseMaximallySizedCerts,
                                               mAnchorIntermediateIssuer.Pubkey(), mIssuer, icac));
        }

        VerifyOrReturnError(CanCastTo<uint16_t>(icac.size()), CHIP_ERROR_INTERNAL);

        // Re-extract DN based on final generated cert
        icac_dn = ChipDN{};
        ReturnErrorOnFailure(ExtractSubjectDNFromX509Cert(icac, icac_dn));

        if (!isForAnchorIcac)
        {
            PERSISTENT_KEY_OP(
                mIndex, kOperationalCredentialsIntermediateCertificateStorage, key,
                ReturnErrorOnFailure(mStorage->SyncSetKeyValue(key, icac.data(), static_cast<uint16_t>(icac.size()))));
        }
        else
        {
            PERSISTENT_KEY_OP(
                mIndex, kOperationalCredentialsAnchorIntermediateCertificateStorage, key,
                ReturnErrorOnFailure(mStorage->SyncSetKeyValue(key, icac.data(), static_cast<uint16_t>(icac.size()))));
        }
    }

    ChipDN noc_dn;
    ReturnErrorOnFailure(noc_dn.AddAttribute_MatterFabricId(fabricId));
    ReturnErrorOnFailure(noc_dn.AddAttribute_MatterNodeId(nodeId));
    ReturnErrorOnFailure(noc_dn.AddCATs(cats));

    ChipLogProgress(Controller, "Generating NOC");
    return IssueX509Cert(mNow, mValidity, icac_dn, noc_dn, CertType::kNoc, mUseMaximallySizedCerts, pubkey,
                         (isForAnchorIcac ? mAnchorIntermediateIssuer : mIntermediateIssuer), noc);
}

CHIP_ERROR
ExampleOperationalCredentialsIssuer::GenerateNOCChain(const ByteSpan & csrElements, const ByteSpan & csrNonce,
                                                      const ByteSpan & attestationSignature, const ByteSpan & attestationChallenge,
                                                      const ByteSpan & DAC, const ByteSpan & PAI,
                                                      Callback::Callback<chip::Controller::OnNOCChainGeneration> * onCompletion)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);
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

    ReturnErrorOnFailure(reader.Expect(kTLVType_Structure, AnonymousTag()));

    TLVType containerType;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    ReturnErrorOnFailure(reader.Next(kTLVType_ByteString, TLV::ContextTag(1)));

    ByteSpan csr(reader.GetReadPoint(), reader.GetLength());
    ReturnErrorOnFailure(reader.ExitContainer(containerType));

    P256PublicKey pubkey;
    ReturnErrorOnFailure(VerifyCertificateSigningRequest(csr.data(), csr.size(), pubkey));

    chip::Platform::ScopedMemoryBuffer<uint8_t> noc;
    VerifyOrReturnError(noc.Alloc(kMaxDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan nocSpan(noc.Get(), kMaxDERCertLength);

    chip::Platform::ScopedMemoryBuffer<uint8_t> icac;
    VerifyOrReturnError(icac.Alloc(kMaxDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan icacSpan(icac.Get(), kMaxDERCertLength);

    chip::Platform::ScopedMemoryBuffer<uint8_t> rcac;
    VerifyOrReturnError(rcac.Alloc(kMaxDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan rcacSpan(rcac.Get(), kMaxDERCertLength);

    if (mHasJFMaterials)
    {
        // Use the cross-signed ICAC and Fabric-A RCAC received from JFA-A.
        // The NOC is signed by mAnchorIntermediateIssuer (whose public key is in the cross-signed ICAC).
        ChipLogProgress(Controller, "GenerateNOCChain: using JF materials from JFA-A (ICAC=%u, RCAC=%u bytes)",
                        static_cast<unsigned>(mJFMaterialsICACLen), static_cast<unsigned>(mJFMaterialsRCACLen));

        // The controller (AutoCommissioner::NOCChainGenerated) unconditionally treats the returned
        // RCAC/ICAC/NOC as DER X509 and converts them to CHIP TLV. The JF materials received over the
        // AddTrustedRootCertificate / AddICAC cluster commands are stored in CHIP TLV format, so they
        // must be converted back to DER X509 before being returned. Do NOT reduce_size the output
        // spans before converting; ConvertChipCertToX509Cert writes DER and shrinks the span itself.
        {
            ByteSpan rcacTlv(mJFMaterialsRCAC, mJFMaterialsRCACLen);
            ReturnErrorOnFailure(ConvertChipCertToX509Cert(rcacTlv, rcacSpan));
        }
        {
            ByteSpan icacTlv(mJFMaterialsICAC, mJFMaterialsICACLen);
            ReturnErrorOnFailure(ConvertChipCertToX509Cert(icacTlv, icacSpan));
        }

        // Extract the ICAC DN to use as issuer for the NOC. The stored JF-materials ICAC is in CHIP TLV
        // format (received over the AddICAC cluster command), so use the CHIP-cert DN extractor on the
        // original TLV bytes (not the DER output span).
        ChipDN icac_dn;
        {
            ByteSpan icacTlv(mJFMaterialsICAC, mJFMaterialsICACLen);
            ReturnErrorOnFailure(ExtractSubjectDNFromChipCert(icacTlv, icac_dn));
        }

        // Extract the fabricId from the cross-signed ICAC.
        FabricId anchorFabricId;
        ReturnErrorOnFailure(icac_dn.GetCertFabricId(anchorFabricId));

        ChipDN noc_dn;
        ReturnErrorOnFailure(noc_dn.AddAttribute_MatterFabricId(anchorFabricId));
        ReturnErrorOnFailure(noc_dn.AddAttribute_MatterNodeId(assignedId));
        ReturnErrorOnFailure(noc_dn.AddCATs(mNextCATs));

        ReturnErrorOnFailure(IssueX509Cert(mNow, mValidity, icac_dn, noc_dn, CertType::kNoc, mUseMaximallySizedCerts, pubkey,
                                           mAnchorIntermediateIssuer, nocSpan));
    }
    else
    {
        ReturnErrorOnFailure(
            GenerateNOCChainAfterValidation(assignedId, mNextFabricId, mNextCATs, pubkey, rcacSpan, icacSpan, nocSpan));
    }

    // The below static assert validates a key assumption in types used (needed for public API conformance)
    static_assert(CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES == kAES_CCM128_Key_Length, "IPK span sizing must match");

    // Use the Fabric-A IPK received from JFA-A if available; otherwise fall back to the default test IPK.
    uint8_t ipkValue[CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];
    Crypto::IdentityProtectionKeySpan ipkSpan(ipkValue);

    if (mHasJFMaterials && mJFMaterialsIPKLen == CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES)
    {
        memcpy(ipkValue, mJFMaterialsIPK, CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES);
        ChipLogProgress(Controller, "GenerateNOCChain: using Fabric-A IPK from JF materials");
    }
    else
    {
        ByteSpan defaultIpkSpan = chip::GroupTesting::DefaultIpkValue::GetDefaultIpk();
        VerifyOrReturnError(defaultIpkSpan.size() == sizeof(ipkValue), CHIP_ERROR_INTERNAL);
        memcpy(ipkValue, defaultIpkSpan.data(), defaultIpkSpan.size());
    }

    // Callback onto commissioner.
    ChipLogProgress(Controller, "Providing certificate chain to the commissioner");
    onCompletion->mCall(onCompletion->mContext, CHIP_NO_ERROR, nocSpan, icacSpan, rcacSpan, MakeOptional(ipkSpan),
                        mNextCaseAdminSubject == kUndefinedNodeId ? Optional<NodeId>() : MakeOptional(mNextCaseAdminSubject));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ExampleOperationalCredentialsIssuer::SignICAC(const ByteSpan & icaCsr, FabricId anchorFabricId, MutableByteSpan & icac)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);

    chip::Platform::ScopedMemoryBuffer<uint8_t> rcac;
    VerifyOrReturnError(rcac.Alloc(kMaxDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan rcacSpan(rcac.Get(), kMaxDERCertLength);

    ChipDN rcac_dn;
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(Controller, "SignICAC: anchorFabricId=0x%016" PRIx64 " csrSize=%u", anchorFabricId,
                    static_cast<unsigned>(icaCsr.size()));

    P256PublicKey pubkey;
    err = VerifyCertificateSigningRequest(icaCsr.data(), icaCsr.size(), pubkey);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "SignICAC: VerifyCertificateSigningRequest failed: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    uint16_t rcacBufLen = static_cast<uint16_t>(std::min(rcacSpan.size(), static_cast<size_t>(UINT16_MAX)));
    PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsRootCertificateStorage, key,
                      err = mStorage->SyncGetKeyValue(key, rcacSpan.data(), rcacBufLen));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "SignICAC: failed to load RCAC from storage: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    {
        uint64_t rcacId;
        // Found root certificate in the storage.
        rcacSpan.reduce_size(rcacBufLen);
        ChipLogProgress(Controller, "SignICAC: RCAC loaded, size=%u", static_cast<unsigned>(rcacSpan.size()));
        err = ExtractSubjectDNFromX509Cert(rcacSpan, rcac_dn);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "SignICAC: ExtractSubjectDNFromX509Cert failed: %" CHIP_ERROR_FORMAT, err.Format());
            return err;
        }
        err = rcac_dn.GetCertChipId(rcacId);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "SignICAC: GetCertChipId failed: %" CHIP_ERROR_FORMAT, err.Format());
            return err;
        }
        ChipLogProgress(Controller, "SignICAC: RCAC issuerId=0x%016" PRIx64 " mIssuerId=0x%08" PRIx32, rcacId,
                        static_cast<uint32_t>(mIssuerId));
        VerifyOrReturnError(rcacId == mIssuerId, CHIP_ERROR_INTERNAL);
    }

    ChipDN icac_dn;
    ReturnErrorOnFailure(icac_dn.AddAttribute_MatterFabricId(anchorFabricId));
    ReturnErrorOnFailure(icac_dn.AddAttribute_MatterICACId(mIntermediateIssuerId));

    ChipLogProgress(Controller, "SignICAC: issuing ICAC with intermediateIssuerId=0x%08" PRIx32,
                    static_cast<uint32_t>(mIntermediateIssuerId));

    // IssueX509Cert produces a DER-encoded cert; the AddICAC cluster command
    // expects CHIP TLV format, so convert before returning.
    Platform::ScopedMemoryBuffer<uint8_t> derBuf;
    VerifyOrReturnError(derBuf.Alloc(kMaxDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan derSpan(derBuf.Get(), kMaxDERCertLength);

    err = IssueX509Cert(mNow, mValidity, rcac_dn, icac_dn, CertType::kIcac, mUseMaximallySizedCerts, pubkey, mIssuer, derSpan);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "SignICAC: IssueX509Cert failed: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    ChipLogProgress(Controller, "SignICAC: ICAC DER size=%u, converting to CHIP TLV", static_cast<unsigned>(derSpan.size()));

    err = ConvertX509CertToChipCert(derSpan, icac);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "SignICAC: ConvertX509CertToChipCert failed: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    ChipLogProgress(Controller, "SignICAC: ICAC CHIP TLV size=%u", static_cast<unsigned>(icac.size()));

    VerifyOrReturnError(CanCastTo<uint16_t>(icac.size()), CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExampleOperationalCredentialsIssuer::SignNOC(const ByteSpan & icac, const ByteSpan & nocCsr, MutableByteSpan & noc)
{
    P256PublicKey pubkey;
    ReturnErrorOnFailure(VerifyCertificateSigningRequest(nocCsr.data(), nocCsr.size(), pubkey));

    ChipDN icac_dn = ChipDN{};
    ReturnErrorOnFailure(ExtractSubjectDNFromX509Cert(icac, icac_dn));

    FabricId anchorFabricId;
    ReturnErrorOnFailure(icac_dn.GetCertFabricId(anchorFabricId));

    ChipDN noc_dn;
    ReturnErrorOnFailure(noc_dn.AddAttribute_MatterFabricId(anchorFabricId));
    ReturnErrorOnFailure(noc_dn.AddAttribute_MatterNodeId(mNextRequestedNodeId));
    ReturnErrorOnFailure(noc_dn.AddCATs(mNextCATs));

    ChipLogProgress(Controller, "Generating NOC");
    return IssueX509Cert(mNow, mValidity, icac_dn, noc_dn, CertType::kNoc, mUseMaximallySizedCerts, pubkey, mIntermediateIssuer,
                         noc);
}

CHIP_ERROR ExampleOperationalCredentialsIssuer::SignDeviceNOC(const ByteSpan & nocCsr, FabricId anchorFabricId, NodeId deviceNodeId,
                                                              MutableByteSpan & noc)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);
    VerifyOrReturnError(mHasJFMaterials, CHIP_ERROR_INCORRECT_STATE);

    ChipLogProgress(Controller, "SignDeviceNOC: anchorFabricId=0x%016" PRIx64 " deviceNodeId=0x%016" PRIx64 " csrSize=%u",
                    anchorFabricId, deviceNodeId, static_cast<unsigned>(nocCsr.size()));

    P256PublicKey pubkey;
    ReturnErrorOnFailure(VerifyCertificateSigningRequest(nocCsr.data(), nocCsr.size(), pubkey));

    // Use the cross-signed JF-materials ICAC as the issuer for the device NOC. Its subject DN carries
    // the Fabric-A fabric id and the ICAC id, and its public key corresponds to mAnchorIntermediateIssuer.
    // The ICAC was received over the AddICAC cluster command in CHIP TLV format (not DER X509), so the
    // subject DN must be extracted with the CHIP-cert helper.
    ByteSpan icacSpan(mJFMaterialsICAC, mJFMaterialsICACLen);
    ChipDN icac_dn;
    ReturnErrorOnFailure(ExtractSubjectDNFromChipCert(icacSpan, icac_dn));

    // The Fabric-A fabric id for the device NOC is derived internally from the cross-signed ICAC.
    // The caller no longer transmits it; the anchorFabricId parameter is ignored.
    (void) anchorFabricId;
    FabricId icacFabricId;
    ReturnErrorOnFailure(icac_dn.GetCertFabricId(icacFabricId));
    ChipLogProgress(Controller, "SignDeviceNOC: derived anchorFabricId=0x%016" PRIx64 " from cross-signed ICAC", icacFabricId);

    ChipDN noc_dn;
    ReturnErrorOnFailure(noc_dn.AddAttribute_MatterFabricId(icacFabricId));

    ReturnErrorOnFailure(noc_dn.AddAttribute_MatterNodeId(deviceNodeId));
    ReturnErrorOnFailure(noc_dn.AddCATs(mNextCATs));

    // IssueX509Cert produces a DER-encoded cert; the AddNOC cluster command expects CHIP TLV, so convert.
    Platform::ScopedMemoryBuffer<uint8_t> derBuf;
    VerifyOrReturnError(derBuf.Alloc(kMaxDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan derSpan(derBuf.Get(), kMaxDERCertLength);

    ChipLogProgress(Controller, "SignDeviceNOC: issuing device NOC signed by anchor intermediate issuer");
    ReturnErrorOnFailure(IssueX509Cert(mNow, mValidity, icac_dn, noc_dn, CertType::kNoc, mUseMaximallySizedCerts, pubkey,
                                       mAnchorIntermediateIssuer, derSpan));

    ReturnErrorOnFailure(ConvertX509CertToChipCert(derSpan, noc));
    VerifyOrReturnError(CanCastTo<uint16_t>(noc.size()), CHIP_ERROR_INTERNAL);

    ChipLogProgress(Controller, "SignDeviceNOC: device NOC CHIP TLV size=%u", static_cast<unsigned>(noc.size()));
    return CHIP_NO_ERROR;
}

void ExampleOperationalCredentialsIssuer::SetJFMaterials(chip::ByteSpan icac, chip::ByteSpan rcac, chip::ByteSpan ipk)

{
    VerifyOrReturn(icac.size() <= sizeof(mJFMaterialsICAC),
                   ChipLogError(Controller, "SetJFMaterials: ICAC too large (%u bytes)", static_cast<unsigned>(icac.size())));
    VerifyOrReturn(rcac.size() <= sizeof(mJFMaterialsRCAC),
                   ChipLogError(Controller, "SetJFMaterials: RCAC too large (%u bytes)", static_cast<unsigned>(rcac.size())));
    VerifyOrReturn(ipk.size() <= sizeof(mJFMaterialsIPK),
                   ChipLogError(Controller, "SetJFMaterials: IPK too large (%u bytes)", static_cast<unsigned>(ipk.size())));

    memcpy(mJFMaterialsICAC, icac.data(), icac.size());
    mJFMaterialsICACLen = icac.size();
    memcpy(mJFMaterialsRCAC, rcac.data(), rcac.size());
    mJFMaterialsRCACLen = rcac.size();
    memcpy(mJFMaterialsIPK, ipk.data(), ipk.size());
    mJFMaterialsIPKLen = ipk.size();
    mHasJFMaterials    = true;

    // Persist JF materials to storage so they survive a reboot.
    if (mStorage != nullptr)
    {
        CHIP_ERROR err;
        err = mStorage->SyncSetKeyValue(kJFMaterialsICACStorage, mJFMaterialsICAC, static_cast<uint16_t>(mJFMaterialsICACLen));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "SetJFMaterials: failed to persist ICAC: %" CHIP_ERROR_FORMAT, err.Format());
        }
        err = mStorage->SyncSetKeyValue(kJFMaterialsRCACStorage, mJFMaterialsRCAC, static_cast<uint16_t>(mJFMaterialsRCACLen));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "SetJFMaterials: failed to persist RCAC: %" CHIP_ERROR_FORMAT, err.Format());
        }
        err = mStorage->SyncSetKeyValue(kJFMaterialsIPKStorage, mJFMaterialsIPK, static_cast<uint16_t>(mJFMaterialsIPKLen));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "SetJFMaterials: failed to persist IPK: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }

    ChipLogProgress(Controller, "SetJFMaterials: stored ICAC=%u bytes, RCAC=%u bytes, IPK=%u bytes",
                    static_cast<unsigned>(icac.size()), static_cast<unsigned>(rcac.size()), static_cast<unsigned>(ipk.size()));
}

void ExampleOperationalCredentialsIssuer::ClearJFMaterials()
{
    mHasJFMaterials     = false;
    mJFMaterialsICACLen = 0;
    mJFMaterialsRCACLen = 0;
    mJFMaterialsIPKLen  = 0;

    // Remove persisted JF materials from storage.
    if (mStorage != nullptr)
    {
        CHIP_ERROR err;
        err = mStorage->SyncDeleteKeyValue(kJFMaterialsICACStorage);
        if (err != CHIP_NO_ERROR && err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            ChipLogError(Controller, "ClearJFMaterials: failed to delete ICAC: %" CHIP_ERROR_FORMAT, err.Format());
        }
        err = mStorage->SyncDeleteKeyValue(kJFMaterialsRCACStorage);
        if (err != CHIP_NO_ERROR && err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            ChipLogError(Controller, "ClearJFMaterials: failed to delete RCAC: %" CHIP_ERROR_FORMAT, err.Format());
        }
        err = mStorage->SyncDeleteKeyValue(kJFMaterialsIPKStorage);
        if (err != CHIP_NO_ERROR && err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            ChipLogError(Controller, "ClearJFMaterials: failed to delete IPK: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }
}

CHIP_ERROR ExampleOperationalCredentialsIssuer::ObtainICACSR(MutableByteSpan & icaCsr)
{
    return GenerateCertificateSigningRequest(&mAnchorIntermediateIssuer, icaCsr);
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

} // namespace CustomCredentialsIssuer
} // namespace chip
