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

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/SafeInt.h>
#include <transport/FabricTable.h>
#if CHIP_CRYPTO_HSM
#include <crypto/hsm/CHIPCryptoPALHsm.h>
#endif

namespace chip {
using namespace Credentials;
using namespace Crypto;

namespace Transport {

CHIP_ERROR FabricInfo::SetFabricLabel(const uint8_t * fabricLabel)
{
    const char * charFabricLabel = Uint8::to_const_char(fabricLabel);
    size_t stringLength          = strnlen(charFabricLabel, kFabricLabelMaxLengthInBytes);
    memcpy(mFabricLabel, charFabricLabel, stringLength);
    mFabricLabel[stringLength] = '\0'; // Set null terminator

    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricInfo::StoreIntoKVS(PersistentStorageDelegate * kvs)
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

    if (mRootCert == nullptr || mRootCertLen == 0)
    {
        info->mRootCertLen = 0;
    }
    else
    {
        info->mRootCertLen = Encoding::LittleEndian::HostSwap16(mRootCertLen);
        memcpy(info->mRootCert, mRootCert, mRootCertLen);
    }

    if (mOperationalCerts == nullptr || mOperationalCertsLen == 0)
    {
        info->mOperationalCertsLen = 0;
    }
    else
    {
        info->mOperationalCertsLen = Encoding::LittleEndian::HostSwap16(mOperationalCertsLen);
        memcpy(info->mOperationalCerts, mOperationalCerts, mOperationalCertsLen);
    }

    err = kvs->SyncSetKeyValue(key, info, sizeof(StorableFabricInfo));
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

CHIP_ERROR FabricInfo::FetchFromKVS(PersistentStorageDelegate * kvs)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    char key[kKeySize];
    ReturnErrorOnFailure(GenerateKey(mFabric, key, sizeof(key)));

    StorableFabricInfo * info = chip::Platform::New<StorableFabricInfo>();
    ReturnErrorCodeIf(info == nullptr, CHIP_ERROR_NO_MEMORY);

    uint16_t infoSize = sizeof(StorableFabricInfo);

    uint16_t id;
    uint16_t rootCertLen, opCertsLen;
    size_t stringLength;

    NodeId nodeId;

    SuccessOrExit(err = kvs->SyncGetKeyValue(key, info, infoSize));

    mFabricId   = Encoding::LittleEndian::HostSwap64(info->mFabricId);
    nodeId      = Encoding::LittleEndian::HostSwap64(info->mNodeId);
    id          = Encoding::LittleEndian::HostSwap16(info->mFabric);
    mVendorId   = Encoding::LittleEndian::HostSwap16(info->mVendorId);
    rootCertLen = Encoding::LittleEndian::HostSwap16(info->mRootCertLen);
    opCertsLen  = Encoding::LittleEndian::HostSwap16(info->mOperationalCertsLen);

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

    ChipLogProgress(Inet, "Loading certs from KVS");
    SuccessOrExit(err = SetRootCert(ByteSpan(info->mRootCert, rootCertLen)));

    // The compressed fabric ID doesn't change for a fabric over time.
    // Computing it here will save computational overhead when it's accessed by other
    // parts of the code.
    SuccessOrExit(err = GetCompressedId(mFabricId, nodeId, &mOperationalId));

    SuccessOrExit(err = SetOperationalCertsFromCertArray(ByteSpan(info->mOperationalCerts, opCertsLen)));

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
    ReturnErrorOnFailure(GenerateCompressedFabricId(mRootPubkey, fabricId, compressedFabricIdSpan));

    // Decode compressed fabric ID accounting for endianness, as GenerateCompressedFabricId()
    // returns a binary buffer and is agnostic of usage of the output as an integer type.
    CompressedFabricId compressedFabricId = Encoding::BigEndian::Get64(compressedFabricIdBuf);
    compressedPeerId->SetCompressedFabricId(compressedFabricId);
    compressedPeerId->SetNodeId(nodeId);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricInfo::DeleteFromKVS(PersistentStorageDelegate * kvs, FabricIndex id)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    char key[kKeySize];
    ReturnErrorOnFailure(GenerateKey(id, key, sizeof(key)));

    err = kvs->SyncDeleteKeyValue(key);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Discovery, "Fabric %d is not yet configured", id);
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

CHIP_ERROR FabricInfo::SetEphemeralKey(const P256Keypair * key)
{
    P256SerializedKeypair serialized;
    ReturnErrorOnFailure(key->Serialize(serialized));
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

void FabricInfo::ReleaseRootCert()
{
    if (mRootCert != nullptr)
    {
        chip::Platform::MemoryFree(mRootCert);
    }
    mRootCertAllocatedLen = 0;
    mRootCertLen          = 0;
    mRootCert             = nullptr;
}

CHIP_ERROR FabricInfo::SetRootCert(const ByteSpan & cert)
{
    if (cert.size() == 0)
    {
        ReleaseRootCert();
        return CHIP_NO_ERROR;
    }

    VerifyOrReturnError(cert.size() <= kMaxCHIPCertLength, CHIP_ERROR_INVALID_ARGUMENT);
    if (mRootCertLen != 0 && mRootCertAllocatedLen < cert.size())
    {
        ReleaseRootCert();
    }

    if (mRootCert == nullptr)
    {
        mRootCert = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(cert.size()));
    }
    VerifyOrReturnError(mRootCert != nullptr, CHIP_ERROR_NO_MEMORY);
    VerifyOrReturnError(CanCastTo<uint16_t>(cert.size()), CHIP_ERROR_INVALID_ARGUMENT);
    mRootCertLen = static_cast<uint16_t>(cert.size());

    // Find root key ID
    ChipCertificateData certData;
    ReturnErrorOnFailure(DecodeChipCert(cert, certData));
    VerifyOrReturnError(certData.mAuthKeyId.size() <= sizeof(mRootKeyId), CHIP_ERROR_INVALID_ARGUMENT);

    memcpy(mRootKeyId, certData.mAuthKeyId.data(), certData.mAuthKeyId.size());
    mRootKeyIdLen = certData.mAuthKeyId.size();

    mRootCertAllocatedLen = (mRootCertLen > mRootCertAllocatedLen) ? mRootCertLen : mRootCertAllocatedLen;
    memcpy(mRootCert, cert.data(), mRootCertLen);

    mRootPubkey = P256PublicKey(certData.mPublicKey);

    return CHIP_NO_ERROR;
}

void FabricInfo::ReleaseOperationalCerts()
{
    if (mOperationalCerts != nullptr)
    {
        chip::Platform::MemoryFree(mOperationalCerts);
    }
    mOperationalCertsLen = 0;
    mOperationalCerts    = nullptr;
}

CHIP_ERROR FabricInfo::SetOperationalCertsFromCertArray(const ByteSpan & certArray)
{
    if (certArray.size() == 0)
    {
        ReleaseOperationalCerts();
        return CHIP_NO_ERROR;
    }

    VerifyOrReturnError(certArray.size() <= kMaxCHIPOpCertArrayLength, CHIP_ERROR_INVALID_ARGUMENT);
    if (mOperationalCertsLen != 0)
    {
        ReleaseOperationalCerts();
    }

    VerifyOrReturnError(CanCastTo<uint16_t>(certArray.size()), CHIP_ERROR_INVALID_ARGUMENT);
    if (mOperationalCerts == nullptr)
    {
        mOperationalCerts = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(certArray.size()));
    }
    VerifyOrReturnError(mOperationalCerts != nullptr, CHIP_ERROR_NO_MEMORY);
    mOperationalCertsLen = static_cast<uint16_t>(certArray.size());
    memcpy(mOperationalCerts, certArray.data(), mOperationalCertsLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricInfo::VerifyCredentials(const ByteSpan & opCertArray, ValidationContext & context, PeerId & nocPeerId,
                                         FabricId & fabricId, Crypto::P256PublicKey & nocPubkey) const
{
    // TODO - Optimize credentials verification logic
    //        The certificate chain construction and verification is a compute and memory intensive operation.
    //        It can be optimized by not loading certificate (i.e. rcac) that's local and implicitly trusted.
    //        The FindValidCert() algorithm will need updates to achieve this refactor.
    ByteSpan rcac(mRootCert, mRootCertLen);
    ByteSpan noc, icac;

    ReturnErrorOnFailure(ExtractCertsFromCertArray(opCertArray, noc, icac));

    constexpr uint8_t kMaxNumCertsInOpCreds = 3;
    uint8_t nocCertIndex                    = 1;

    ChipCertificateSet certificates;
    ReturnErrorOnFailure(certificates.Init(kMaxNumCertsInOpCreds));

    ReturnErrorOnFailure(certificates.LoadCert(rcac, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)));

    if (!icac.empty())
    {
        ReturnErrorOnFailure(certificates.LoadCert(icac, BitFlags<CertDecodeFlags>(CertDecodeFlags::kGenerateTBSHash)));
        nocCertIndex = 2;
    }

    ReturnErrorOnFailure(certificates.LoadCert(noc, BitFlags<CertDecodeFlags>(CertDecodeFlags::kGenerateTBSHash)));

    const ChipDN & nocSubjectDN              = certificates.GetCertSet()[nocCertIndex].mSubjectDN;
    const CertificateKeyId & nocSubjectKeyId = certificates.GetCertSet()[nocCertIndex].mSubjectKeyId;

    const ChipCertificateData * resultCert = nullptr;
    // FindValidCert() checks the certificate set constructed by loading noc, icac and rcac.
    // It confirms that the certs link correctly (noc -> icac -> rcac), and have been correctly signed.
    ReturnErrorOnFailure(certificates.FindValidCert(nocSubjectDN, nocSubjectKeyId, context, &resultCert));

    NodeId nodeId;
    ReturnErrorOnFailure(ExtractNodeIdFabricIdFromOpCert(certificates.GetCertSet()[nocCertIndex], &nodeId, &fabricId));

    if (!icac.empty())
    {
        FabricId icacFabric = kUndefinedFabricId;
        if (ExtractFabricIdFromCert(certificates.GetCertSet()[1], &icacFabric) == CHIP_NO_ERROR && icacFabric != kUndefinedFabricId)
        {
            ReturnErrorCodeIf(icacFabric != fabricId, CHIP_ERROR_FABRIC_MISMATCH_ON_ICA);
        }
    }

    ReturnErrorOnFailure(GetCompressedId(fabricId, nodeId, &nocPeerId));
    nocPubkey = P256PublicKey(certificates.GetCertSet()[nocCertIndex].mPublicKey);

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

    Encoding::LittleEndian::BufferWriter bbuf(destinationMessage, sizeof(destinationMessage));

    bbuf.Put(random.data(), random.size());
    bbuf.Put(mRootPubkey.ConstBytes(), mRootPubkey.Length());
    bbuf.Put64(mFabricId);
    bbuf.Put64(destNodeId);

    size_t written = 0;
    VerifyOrReturnError(bbuf.Fit(written), CHIP_ERROR_BUFFER_TOO_SMALL);

    CHIP_ERROR err =
        hmac.HMAC_SHA256(ipk.data(), ipk.size(), destinationMessage, written, destinationId.data(), destinationId.size());
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

CHIP_ERROR FabricTable::Store(FabricIndex id)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    FabricInfo * fabric = nullptr;

    VerifyOrExit(mStorage != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    fabric = FindFabricWithIndex(id);
    VerifyOrExit(fabric != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    err = fabric->StoreIntoKVS(mStorage);
exit:
    if (err == CHIP_NO_ERROR && mDelegate != nullptr)
    {
        ChipLogProgress(Discovery, "Fabric (%d) persisted to storage. Calling OnFabricPersistedToStorage", id);
        mDelegate->OnFabricPersistedToStorage(fabric);
    }
    return err;
}

CHIP_ERROR FabricTable::LoadFromStorage(FabricInfo * fabric)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    if (!fabric->IsInitialized())
    {
        ReturnErrorOnFailure(fabric->FetchFromKVS(mStorage));
    }

    if (mDelegate != nullptr)
    {
        ChipLogProgress(Discovery, "Fabric (%d) loaded from storage. Calling OnFabricRetrievedFromStorage",
                        fabric->GetFabricIndex());
        mDelegate->OnFabricRetrievedFromStorage(fabric);
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

    SetEphemeralKey(newFabric.GetOperationalKey());
    SetRootCert(ByteSpan(newFabric.mRootCert, newFabric.mRootCertLen));

    ChipLogProgress(Discovery, "Verifying the received credentials");
    ReturnErrorOnFailure(VerifyCredentials(ByteSpan(newFabric.mOperationalCerts, newFabric.mOperationalCertsLen), validContext,
                                           mOperationalId, mFabricId, pubkey));

    SetOperationalCertsFromCertArray(ByteSpan(newFabric.mOperationalCerts, newFabric.mOperationalCertsLen));
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
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_NO_MEMORY;
}

CHIP_ERROR FabricTable::Delete(FabricIndex id)
{
    FabricInfo * fabric      = nullptr;
    CHIP_ERROR err           = CHIP_NO_ERROR;
    bool fabricIsInitialized = false;
    VerifyOrExit(mStorage != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    fabric              = FindFabricWithIndex(id);
    fabricIsInitialized = fabric != nullptr && fabric->IsInitialized();
    err                 = FabricInfo::DeleteFromKVS(mStorage, id); // Delete from storage regardless

exit:
    if (err == CHIP_NO_ERROR)
    {
        ReleaseFabricIndex(id);
        if (mDelegate != nullptr && fabricIsInitialized)
        {
            ChipLogProgress(Discovery, "Fabric (%d) deleted. Calling OnFabricDeletedFromStorage", id);
            mDelegate->OnFabricDeletedFromStorage(id);
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

CHIP_ERROR FabricTable::Init(PersistentStorageDelegate * storage)
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
        LoadFromStorage(fabric);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricTable::SetFabricDelegate(FabricTableDelegate * delegate)
{
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mDelegate = delegate;
    ChipLogDetail(Discovery, "Set the fabric pairing table delegate");
    return CHIP_NO_ERROR;
}

} // namespace Transport
} // namespace chip
