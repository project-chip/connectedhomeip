/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 * @brief Defines a table of fabrics that have provisioned the device.
 */

#include "FabricTable.h"

#include <lib/core/CHIPEncoding.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/SafeInt.h>
#if CHIP_CRYPTO_HSM
#include <crypto/hsm/CHIPCryptoPALHsm.h>
#endif

namespace chip {
using namespace Credentials;
using namespace Crypto;

CHIP_ERROR FabricInfo::SetFabricLabel(const CharSpan & fabricLabel)
{
    Platform::CopyString(mFabricLabel, fabricLabel);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricInfo::CommitToStorage(FabricStorage * storage)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    char key[kKeySize];
    ReturnErrorOnFailure(GenerateKey(mFabric, key, sizeof(key)));

    StorableFabricInfo * info = chip::Platform::New<StorableFabricInfo>();
    ReturnErrorCodeIf(info == nullptr, CHIP_ERROR_NO_MEMORY);

    info->mNodeId   = Encoding::LittleEndian::HostSwap64(mOperationalId.GetNodeId());
    info->mFabric   = Encoding::LittleEndian::HostSwap16(mFabric);
    info->mVendorId = Encoding::LittleEndian::HostSwap16(mVendorId);

    info->mFabricId = Encoding::LittleEndian::HostSwap64(mFabricId);

    size_t stringLength = strnlen(mFabricLabel, kFabricLabelMaxLengthInBytes);
    memcpy(info->mFabricLabel, mFabricLabel, stringLength);
    info->mFabricLabel[stringLength] = '\0'; // Set null terminator

    if (mOperationalKey != nullptr)
    {
        SuccessOrExit(err = mOperationalKey->Serialize(info->mOperationalKey));
    }
    else
    {
        P256Keypair keypair;
        SuccessOrExit(err = keypair.Initialize());
        SuccessOrExit(err = keypair.Serialize(info->mOperationalKey));
    }

    if (mRootCert.empty())
    {
        info->mRootCertLen = 0;
    }
    else
    {
        VerifyOrExit(CanCastTo<uint16_t>(mRootCert.size()), err = CHIP_ERROR_INVALID_ARGUMENT);
        info->mRootCertLen = Encoding::LittleEndian::HostSwap16(static_cast<uint16_t>(mRootCert.size()));
        memcpy(info->mRootCert, mRootCert.data(), mRootCert.size());
    }

    if (mICACert.empty())
    {
        info->mICACertLen = 0;
    }
    else
    {
        VerifyOrExit(CanCastTo<uint16_t>(mICACert.size()), err = CHIP_ERROR_INVALID_ARGUMENT);
        info->mICACertLen = Encoding::LittleEndian::HostSwap16(static_cast<uint16_t>(mICACert.size()));
        memcpy(info->mICACert, mICACert.data(), mICACert.size());
    }

    if (mNOCCert.empty())
    {
        info->mNOCCertLen = 0;
    }
    else
    {
        VerifyOrExit(CanCastTo<uint16_t>(mNOCCert.size()), err = CHIP_ERROR_INVALID_ARGUMENT);
        info->mNOCCertLen = Encoding::LittleEndian::HostSwap16(static_cast<uint16_t>(mNOCCert.size()));
        memcpy(info->mNOCCert, mNOCCert.data(), mNOCCert.size());
    }

    err = storage->SyncStore(mFabric, key, info, sizeof(StorableFabricInfo));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Error occurred calling SyncSetKeyValue: %s", chip::ErrorStr(err));
    }

exit:
    if (info != nullptr)
    {
        chip::Platform::Delete(info);
    }
    return err;
}

CHIP_ERROR FabricInfo::LoadFromStorage(FabricStorage * storage)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    char key[kKeySize];
    ReturnErrorOnFailure(GenerateKey(mFabric, key, sizeof(key)));

    StorableFabricInfo * info = chip::Platform::New<StorableFabricInfo>();
    ReturnErrorCodeIf(info == nullptr, CHIP_ERROR_NO_MEMORY);

    uint16_t infoSize = sizeof(StorableFabricInfo);

    uint16_t id;
    uint16_t rootCertLen, icaCertLen, nocCertLen;
    size_t stringLength;

    NodeId nodeId;

    SuccessOrExit(err = storage->SyncLoad(mFabric, key, info, infoSize));

    mFabricId   = Encoding::LittleEndian::HostSwap64(info->mFabricId);
    nodeId      = Encoding::LittleEndian::HostSwap64(info->mNodeId);
    id          = Encoding::LittleEndian::HostSwap16(info->mFabric);
    mVendorId   = Encoding::LittleEndian::HostSwap16(info->mVendorId);
    rootCertLen = Encoding::LittleEndian::HostSwap16(info->mRootCertLen);
    icaCertLen  = Encoding::LittleEndian::HostSwap16(info->mICACertLen);
    nocCertLen  = Encoding::LittleEndian::HostSwap16(info->mNOCCertLen);

    stringLength = strnlen(info->mFabricLabel, kFabricLabelMaxLengthInBytes);
    memcpy(mFabricLabel, info->mFabricLabel, stringLength);
    mFabricLabel[stringLength] = '\0'; // Set null terminator

    VerifyOrExit(mFabric == id, err = CHIP_ERROR_INCORRECT_STATE);

    if (mOperationalKey == nullptr)
    {
#ifdef ENABLE_HSM_CASE_OPS_KEY
        mOperationalKey = chip::Platform::New<P256KeypairHSM>();
        mOperationalKey->SetKeyId(CASE_OPS_KEY);
#else
        mOperationalKey = chip::Platform::New<P256Keypair>();
#endif
    }
    VerifyOrExit(mOperationalKey != nullptr, err = CHIP_ERROR_NO_MEMORY);
    SuccessOrExit(err = mOperationalKey->Deserialize(info->mOperationalKey));
#ifdef ENABLE_HSM_CASE_OPS_KEY
    // Set provisioned_key = true , so that key is not deleted from HSM.
    mOperationalKey->provisioned_key = true;
#endif

    ChipLogProgress(Inet, "Loading certs from storage");
    SuccessOrExit(err = SetRootCert(ByteSpan(info->mRootCert, rootCertLen)));

    // The compressed fabric ID doesn't change for a fabric over time.
    // Computing it here will save computational overhead when it's accessed by other
    // parts of the code.
    SuccessOrExit(err = GetCompressedId(mFabricId, nodeId, &mOperationalId));

    SuccessOrExit(err = SetICACert(ByteSpan(info->mICACert, icaCertLen)));
    SuccessOrExit(err = SetNOCCert(ByteSpan(info->mNOCCert, nocCertLen)));

exit:
    if (info != nullptr)
    {
        chip::Platform::Delete(info);
    }
    return err;
}

CHIP_ERROR FabricInfo::GetCompressedId(FabricId fabricId, NodeId nodeId, PeerId * compressedPeerId) const
{
    ReturnErrorCodeIf(compressedPeerId == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    uint8_t compressedFabricIdBuf[sizeof(uint64_t)];
    MutableByteSpan compressedFabricIdSpan(compressedFabricIdBuf);
    P256PublicKey rootPubkey;

    {
        P256PublicKeySpan rootPubkeySpan;
        ReturnErrorOnFailure(GetRootPubkey(rootPubkeySpan));
        rootPubkey = rootPubkeySpan;
    }

    ChipLogDetail(Inet, "Generating compressed fabric ID using uncompressed fabric ID 0x" ChipLogFormatX64 " and root pubkey",
                  ChipLogValueX64(fabricId));
    ChipLogByteSpan(Inet, ByteSpan(rootPubkey.ConstBytes(), rootPubkey.Length()));
    ReturnErrorOnFailure(GenerateCompressedFabricId(rootPubkey, fabricId, compressedFabricIdSpan));
    ChipLogDetail(Inet, "Generated compressed fabric ID");
    ChipLogByteSpan(Inet, compressedFabricIdSpan);

    // Decode compressed fabric ID accounting for endianness, as GenerateCompressedFabricId()
    // returns a binary buffer and is agnostic of usage of the output as an integer type.
    CompressedFabricId compressedFabricId = Encoding::BigEndian::Get64(compressedFabricIdBuf);
    compressedPeerId->SetCompressedFabricId(compressedFabricId);
    compressedPeerId->SetNodeId(nodeId);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricInfo::DeleteFromStorage(FabricStorage * storage, FabricIndex fabricIndex)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    char key[kKeySize];
    ReturnErrorOnFailure(GenerateKey(fabricIndex, key, sizeof(key)));

    err = storage->SyncDelete(fabricIndex, key);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Discovery, "Fabric %d is not yet configured", fabricIndex);
    }
    return err;
}

CHIP_ERROR FabricInfo::GenerateKey(FabricIndex id, char * key, size_t len)
{
    VerifyOrReturnError(len >= kKeySize, CHIP_ERROR_INVALID_ARGUMENT);
    int keySize = snprintf(key, len, "%s%x", kFabricTableKeyPrefix, id);
    VerifyOrReturnError(keySize > 0, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(len > (size_t) keySize, CHIP_ERROR_INTERNAL);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricInfo::SetOperationalKeypair(const P256Keypair * keyPair)
{
    VerifyOrReturnError(keyPair != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    P256SerializedKeypair serialized;
    ReturnErrorOnFailure(keyPair->Serialize(serialized));
    if (mOperationalKey == nullptr)
    {
#ifdef ENABLE_HSM_CASE_OPS_KEY
        mOperationalKey = chip::Platform::New<P256KeypairHSM>();
        mOperationalKey->SetKeyId(CASE_OPS_KEY);
#else
        mOperationalKey = chip::Platform::New<P256Keypair>();
#endif
    }
    VerifyOrReturnError(mOperationalKey != nullptr, CHIP_ERROR_NO_MEMORY);
    return mOperationalKey->Deserialize(serialized);
}

void FabricInfo::ReleaseCert(MutableByteSpan & cert)
{
    if (cert.data() != nullptr)
    {
        chip::Platform::MemoryFree(cert.data());
    }
    cert = MutableByteSpan();
}

CHIP_ERROR FabricInfo::SetCert(MutableByteSpan & dstCert, const ByteSpan & srcCert)
{
    ReleaseCert(dstCert);
    if (srcCert.data() == nullptr || srcCert.size() == 0)
    {
        return CHIP_NO_ERROR;
    }

    VerifyOrReturnError(srcCert.size() <= kMaxCHIPCertLength, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(CanCastTo<uint16_t>(srcCert.size()), CHIP_ERROR_INVALID_ARGUMENT);

    dstCert = MutableByteSpan(static_cast<uint8_t *>(chip::Platform::MemoryAlloc(srcCert.size())), srcCert.size());
    VerifyOrReturnError(dstCert.data() != nullptr, CHIP_ERROR_NO_MEMORY);

    memcpy(dstCert.data(), srcCert.data(), srcCert.size());

    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricInfo::VerifyCredentials(const ByteSpan & noc, const ByteSpan & icac, ValidationContext & context,
                                         PeerId & nocPeerId, FabricId & fabricId, Crypto::P256PublicKey & nocPubkey) const
{
    // TODO - Optimize credentials verification logic
    //        The certificate chain construction and verification is a compute and memory intensive operation.
    //        It can be optimized by not loading certificate (i.e. rcac) that's local and implicitly trusted.
    //        The FindValidCert() algorithm will need updates to achieve this refactor.
    constexpr uint8_t kMaxNumCertsInOpCreds = 3;

    ChipCertificateSet certificates;
    ReturnErrorOnFailure(certificates.Init(kMaxNumCertsInOpCreds));

    ReturnErrorOnFailure(certificates.LoadCert(mRootCert, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)));

    if (!icac.empty())
    {
        ReturnErrorOnFailure(certificates.LoadCert(icac, BitFlags<CertDecodeFlags>(CertDecodeFlags::kGenerateTBSHash)));
    }

    ReturnErrorOnFailure(certificates.LoadCert(noc, BitFlags<CertDecodeFlags>(CertDecodeFlags::kGenerateTBSHash)));

    const ChipDN & nocSubjectDN              = certificates.GetLastCert()[0].mSubjectDN;
    const CertificateKeyId & nocSubjectKeyId = certificates.GetLastCert()[0].mSubjectKeyId;

    const ChipCertificateData * resultCert = nullptr;
    // FindValidCert() checks the certificate set constructed by loading noc, icac and mRootCert.
    // It confirms that the certs link correctly (noc -> icac -> mRootCert), and have been correctly signed.
    ReturnErrorOnFailure(certificates.FindValidCert(nocSubjectDN, nocSubjectKeyId, context, &resultCert));

    NodeId nodeId;
    ReturnErrorOnFailure(ExtractNodeIdFabricIdFromOpCert(certificates.GetLastCert()[0], &nodeId, &fabricId));

    if (!icac.empty())
    {
        FabricId icacFabric = kUndefinedFabricId;
        if (ExtractFabricIdFromCert(certificates.GetCertSet()[1], &icacFabric) == CHIP_NO_ERROR && icacFabric != kUndefinedFabricId)
        {
            ReturnErrorCodeIf(icacFabric != fabricId, CHIP_ERROR_FABRIC_MISMATCH_ON_ICA);
        }
    }

    ReturnErrorOnFailure(GetCompressedId(fabricId, nodeId, &nocPeerId));
    nocPubkey = P256PublicKey(certificates.GetLastCert()[0].mPublicKey);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricInfo::GenerateDestinationID(const ByteSpan & ipk, const ByteSpan & random, NodeId destNodeId,
                                             MutableByteSpan & destinationId)
{
    constexpr uint16_t kSigmaParamRandomNumberSize = 32;
    constexpr size_t kDestinationMessageLen =
        kSigmaParamRandomNumberSize + kP256_PublicKey_Length + sizeof(FabricId) + sizeof(NodeId);
    HMAC_sha hmac;
    uint8_t destinationMessage[kDestinationMessageLen];
    P256PublicKeySpan rootPubkeySpan;

    ReturnErrorOnFailure(GetRootPubkey(rootPubkeySpan));

    Encoding::LittleEndian::BufferWriter bbuf(destinationMessage, sizeof(destinationMessage));

    ChipLogDetail(Inet,
                  "Generating DestinationID. Fabric ID 0x" ChipLogFormatX64 ", Dest node ID 0x" ChipLogFormatX64 ", Random data",
                  ChipLogValueX64(mFabricId), ChipLogValueX64(destNodeId));
    ChipLogByteSpan(Inet, random);

    bbuf.Put(random.data(), random.size());
    // TODO: In the current implementation this check is required because in some cases the
    //       GenerateDestinationID() is called before mRootCert is initialized and GetRootPubkey() returns
    //       empty Span.
    if (!rootPubkeySpan.empty())
    {
        ChipLogDetail(Inet, "Root pubkey");
        ChipLogByteSpan(Inet, rootPubkeySpan);
        bbuf.Put(rootPubkeySpan.data(), rootPubkeySpan.size());
    }
    bbuf.Put64(mFabricId);
    bbuf.Put64(destNodeId);

    size_t written = 0;
    VerifyOrReturnError(bbuf.Fit(written), CHIP_ERROR_BUFFER_TOO_SMALL);

    ChipLogDetail(Inet, "IPK");
    ChipLogByteSpan(Inet, ipk);

    CHIP_ERROR err =
        hmac.HMAC_SHA256(ipk.data(), ipk.size(), destinationMessage, written, destinationId.data(), destinationId.size());
    ChipLogDetail(Inet, "Generated DestinationID output");
    ChipLogByteSpan(Inet, destinationId);
    return err;
}

CHIP_ERROR FabricInfo::MatchDestinationID(const ByteSpan & targetDestinationId, const ByteSpan & initiatorRandom,
                                          const ByteSpan * ipkList, size_t ipkListEntries)
{
    uint8_t localDestID[kSHA256_Hash_Length] = { 0 };
    MutableByteSpan localDestIDSpan(localDestID);
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
    for (size_t ipkIdx = 0; ipkIdx < ipkListEntries; ++ipkIdx)
    {
        if (GenerateDestinationID(ipkList[ipkIdx], initiatorRandom, mOperationalId.GetNodeId(), localDestIDSpan) == CHIP_NO_ERROR &&
            targetDestinationId.data_equal(localDestIDSpan))
        {
            return CHIP_NO_ERROR;
        }
    }
    return CHIP_ERROR_CERT_NOT_TRUSTED;
}

void FabricTable::ReleaseFabricIndex(FabricIndex fabricIndex)
{
    FabricInfo * fabric = FindFabricWithIndex(fabricIndex);
    if (fabric != nullptr)
    {
        fabric->Reset();
    }
}

FabricInfo * FabricTable::FindFabric(P256PublicKeySpan rootPubKey, FabricId fabricId)
{
    static_assert(kMaxValidFabricIndex <= UINT8_MAX, "Cannot create more fabrics than UINT8_MAX");
    for (FabricIndex i = kMinValidFabricIndex; i <= kMaxValidFabricIndex; i++)
    {
        FabricInfo * fabric = FindFabricWithIndex(i);
        if (fabric == nullptr)
        {
            continue;
        }
        P256PublicKeySpan candidatePubKey;
        if (fabric->GetRootPubkey(candidatePubKey) != CHIP_NO_ERROR)
        {
            continue;
        }
        if (rootPubKey.data_equal(candidatePubKey) && fabricId == fabric->GetFabricId())
        {
            LoadFromStorage(fabric);
            return fabric;
        }
    }
    return nullptr;
}

FabricInfo * FabricTable::FindFabricWithIndex(FabricIndex fabricIndex)
{
    if (fabricIndex >= kMinValidFabricIndex && fabricIndex <= kMaxValidFabricIndex)
    {
        FabricInfo * fabric = &mStates[fabricIndex - kMinValidFabricIndex];
        LoadFromStorage(fabric);
        return fabric;
    }

    return nullptr;
}

FabricInfo * FabricTable::FindFabricWithCompressedId(CompressedFabricId fabricId)
{
    static_assert(kMaxValidFabricIndex <= UINT8_MAX, "Cannot create more fabrics than UINT8_MAX");
    for (FabricIndex i = kMinValidFabricIndex; i <= kMaxValidFabricIndex; i++)
    {
        FabricInfo * fabric = FindFabricWithIndex(i);

        if (fabric != nullptr && fabricId == fabric->GetPeerId().GetCompressedFabricId())
        {
            LoadFromStorage(fabric);
            return fabric;
        }
    }
    return nullptr;
}

void FabricTable::Reset()
{
    static_assert(kMaxValidFabricIndex <= UINT8_MAX, "Cannot create more fabrics than UINT8_MAX");
    for (FabricIndex i = kMinValidFabricIndex; i <= kMaxValidFabricIndex; i++)
    {
        FabricInfo * fabric = FindFabricWithIndex(i);

        if (fabric != nullptr)
        {
            fabric->Reset();

            fabric->mFabric = i;
        }
    }
}

CHIP_ERROR FabricTable::Store(FabricIndex index)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    FabricInfo * fabric = nullptr;

    VerifyOrExit(mStorage != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    fabric = FindFabricWithIndex(index);
    VerifyOrExit(fabric != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    err = fabric->CommitToStorage(mStorage);
exit:
    if (err == CHIP_NO_ERROR && mDelegate != nullptr)
    {
        ChipLogProgress(Discovery, "Fabric (%d) persisted to storage. Calling OnFabricPersistedToStorage", index);
        FabricTableDelegate * delegate = mDelegate;
        while (delegate)
        {
            delegate->OnFabricPersistedToStorage(fabric);
            delegate = delegate->mNext;
        }
    }
    return err;
}

CHIP_ERROR FabricTable::LoadFromStorage(FabricInfo * fabric)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    if (!fabric->IsInitialized())
    {
        ReturnErrorOnFailure(fabric->LoadFromStorage(mStorage));
    }

    FabricTableDelegate * delegate = mDelegate;
    while (delegate)
    {
        ChipLogProgress(Discovery, "Fabric (%d) loaded from storage. Calling OnFabricRetrievedFromStorage",
                        fabric->GetFabricIndex());
        delegate->OnFabricRetrievedFromStorage(fabric);
        delegate = delegate->mNext;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricInfo::SetFabricInfo(FabricInfo & newFabric)
{
    P256PublicKey pubkey;
    ValidationContext validContext;
    validContext.Reset();
    validContext.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
    validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kServerAuth);

    SetOperationalKeypair(newFabric.GetOperationalKey());
    SetRootCert(newFabric.mRootCert);

    ChipLogProgress(Discovery, "Verifying the received credentials");
    ReturnErrorOnFailure(
        VerifyCredentials(newFabric.mNOCCert, newFabric.mICACert, validContext, mOperationalId, mFabricId, pubkey));

    SetICACert(newFabric.mICACert);
    SetNOCCert(newFabric.mNOCCert);
    SetVendorId(newFabric.GetVendorId());
    SetFabricLabel(newFabric.GetFabricLabel());
    ChipLogProgress(Discovery, "Added new fabric at index: %d, Initialized: %d", GetFabricIndex(), IsInitialized());
    ChipLogProgress(Discovery, "Assigned compressed fabric ID: 0x" ChipLogFormatX64 ", node ID: 0x" ChipLogFormatX64,
                    ChipLogValueX64(mOperationalId.GetCompressedFabricId()), ChipLogValueX64(mOperationalId.GetNodeId()));
    return CHIP_NO_ERROR;
}

FabricIndex FabricTable::FindDestinationIDCandidate(const ByteSpan & destinationId, const ByteSpan & initiatorRandom,
                                                    const ByteSpan * ipkList, size_t ipkListEntries)
{
    static_assert(kMaxValidFabricIndex <= UINT8_MAX, "Cannot create more fabrics than UINT8_MAX");
    for (FabricIndex i = kMinValidFabricIndex; i <= kMaxValidFabricIndex; i++)
    {
        FabricInfo * fabric = FindFabricWithIndex(i);
        if (fabric != nullptr &&
            fabric->MatchDestinationID(destinationId, initiatorRandom, ipkList, ipkListEntries) == CHIP_NO_ERROR)
        {
            return i;
        }
    }

    return kUndefinedFabricIndex;
}

CHIP_ERROR FabricTable::AddNewFabric(FabricInfo & newFabric, FabricIndex * outputIndex)
{
    VerifyOrReturnError(outputIndex != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    static_assert(kMaxValidFabricIndex <= UINT8_MAX, "Cannot create more fabrics than UINT8_MAX");
    for (FabricIndex i = mNextAvailableFabricIndex; i <= kMaxValidFabricIndex; i++)
    {
        FabricInfo * fabric = FindFabricWithIndex(i);
        if (fabric != nullptr && !fabric->IsInitialized())
        {
            ReturnErrorOnFailure(fabric->SetFabricInfo(newFabric));
            ReturnErrorOnFailure(Store(i));
            mNextAvailableFabricIndex = static_cast<FabricIndex>((i + 1) % UINT8_MAX);
            *outputIndex              = i;
            mFabricCount++;
            return CHIP_NO_ERROR;
        }
    }

    for (FabricIndex i = kMinValidFabricIndex; i < kMaxValidFabricIndex; i++)
    {
        FabricInfo * fabric = FindFabricWithIndex(i);
        if (fabric != nullptr && !fabric->IsInitialized())
        {
            ReturnErrorOnFailure(fabric->SetFabricInfo(newFabric));
            ReturnErrorOnFailure(Store(i));
            mNextAvailableFabricIndex = static_cast<FabricIndex>((i + 1) % UINT8_MAX);
            *outputIndex              = i;
            mFabricCount++;
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_NO_MEMORY;
}

CHIP_ERROR FabricTable::Delete(FabricIndex index)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    FabricInfo * fabric      = FindFabricWithIndex(index);
    bool fabricIsInitialized = fabric != nullptr && fabric->IsInitialized();
    CompressedFabricId compressedFabricId =
        fabricIsInitialized ? fabric->GetPeerId().GetCompressedFabricId() : kUndefinedCompressedFabricId;
    CHIP_ERROR err = FabricInfo::DeleteFromStorage(mStorage, index); // Delete from storage regardless
    if (!fabricIsInitialized)
    {
        // Make sure to return the error our API promises, not whatever storage
        // chose to return.
        return CHIP_ERROR_NOT_FOUND;
    }
    ReturnErrorOnFailure(err);

    ReleaseFabricIndex(index);
    if (mDelegate != nullptr)
    {
        if (mFabricCount == 0)
        {
            ChipLogError(Discovery, "!!Trying to delete a fabric, but the current fabric count is already 0");
        }
        else
        {
            mFabricCount--;
        }
        ChipLogProgress(Discovery, "Fabric (%d) deleted. Calling OnFabricDeletedFromStorage", index);
        FabricTableDelegate * delegate = mDelegate;
        while (delegate)
        {
            delegate->OnFabricDeletedFromStorage(compressedFabricId, index);
            delegate = delegate->mNext;
        }
    }
    return CHIP_NO_ERROR;
}

void FabricTable::DeleteAllFabrics()
{
    static_assert(kMaxValidFabricIndex <= UINT8_MAX, "Cannot create more fabrics than UINT8_MAX");
    for (FabricIndex i = kMinValidFabricIndex; i <= kMaxValidFabricIndex; i++)
    {
        Delete(i);
    }
}

CHIP_ERROR FabricTable::Init(FabricStorage * storage)
{
    VerifyOrReturnError(storage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mStorage = storage;
    ChipLogDetail(Discovery, "Init fabric pairing table with server storage");

    // Load the current fabrics from the storage. This is done here, since ConstFabricIterator
    // iterator doesn't have mechanism to load fabric info from storage on demand.
    // TODO - Update ConstFabricIterator to load fabric info from storage
    static_assert(kMaxValidFabricIndex <= UINT8_MAX, "Cannot create more fabrics than UINT8_MAX");
    for (FabricIndex i = kMinValidFabricIndex; i <= kMaxValidFabricIndex; i++)
    {
        FabricInfo * fabric = &mStates[i - kMinValidFabricIndex];
        if (LoadFromStorage(fabric) == CHIP_NO_ERROR)
        {
            mFabricCount++;
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricTable::AddFabricDelegate(FabricTableDelegate * delegate)
{
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    for (FabricTableDelegate * iter = mDelegate; iter != nullptr; iter = iter->mNext)
    {
        if (iter == delegate)
        {
            return CHIP_NO_ERROR;
        }
    }
    delegate->mNext = mDelegate;
    mDelegate       = delegate;
    ChipLogDetail(Discovery, "Add fabric pairing table delegate");
    return CHIP_NO_ERROR;
}

CHIP_ERROR formatKey(FabricIndex fabricIndex, MutableCharSpan formattedKey, const char * key)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    int res = snprintf(formattedKey.data(), formattedKey.size(), "F%02X/%s", fabricIndex, key);
    if (res < 0 || (size_t) res >= formattedKey.size())
    {
        ChipLogError(Discovery, "Failed to format Key %s. snprintf error: %d", key, res);
        return CHIP_ERROR_NO_MEMORY;
    }
    return err;
}

CHIP_ERROR SimpleFabricStorage::SyncStore(FabricIndex fabricIndex, const char * key, const void * buffer, uint16_t size)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    char formattedKey[MAX_KEY_SIZE] = "";
    ReturnErrorOnFailure(formatKey(fabricIndex, MutableCharSpan(formattedKey, MAX_KEY_SIZE), key));
    return mStorage->SyncSetKeyValue(formattedKey, buffer, size);
};

CHIP_ERROR SimpleFabricStorage::SyncLoad(FabricIndex fabricIndex, const char * key, void * buffer, uint16_t & size)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    char formattedKey[MAX_KEY_SIZE] = "";
    ReturnErrorOnFailure(formatKey(fabricIndex, MutableCharSpan(formattedKey, MAX_KEY_SIZE), key));
    return mStorage->SyncGetKeyValue(formattedKey, buffer, size);
};

CHIP_ERROR SimpleFabricStorage::SyncDelete(FabricIndex fabricIndex, const char * key)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    char formattedKey[MAX_KEY_SIZE] = "";
    ReturnErrorOnFailure(formatKey(fabricIndex, MutableCharSpan(formattedKey, MAX_KEY_SIZE), key));
    return mStorage->SyncDeleteKeyValue(formattedKey);
};

} // namespace chip
