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

#include <core/CHIPEncoding.h>
#include <support/CHIPMem.h>
#include <support/SafeInt.h>
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

    char key[KeySize()];
    ReturnErrorOnFailure(GenerateKey(mFabric, key, sizeof(key)));

    StorableFabricInfo * info = chip::Platform::New<StorableFabricInfo>();
    ReturnErrorCodeIf(info == nullptr, CHIP_ERROR_NO_MEMORY);

    info->mNodeId   = Encoding::LittleEndian::HostSwap64(mOperationalId.GetNodeId());
    info->mFabric   = Encoding::LittleEndian::HostSwap16(mFabric);
    info->mFabricId = Encoding::LittleEndian::HostSwap64(mOperationalId.GetFabricId());
    info->mVendorId = Encoding::LittleEndian::HostSwap16(mVendorId);

    size_t stringLength = strnlen(mFabricLabel, kFabricLabelMaxLengthInBytes);
    memcpy(info->mFabricLabel, mFabricLabel, stringLength);
    info->mFabricLabel[stringLength] = '\0'; // Set null terminator

    if (mEphemeralKey != nullptr)
    {
        SuccessOrExit(err = mEphemeralKey->Serialize(info->mEphemeralKey));
    }
    else
    {
        P256Keypair keypair;
        SuccessOrExit(err = keypair.Initialize());
        SuccessOrExit(err = keypair.Serialize(info->mEphemeralKey));
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

    if (mICACert == nullptr || mICACertLen == 0)
    {
        info->mICACertLen = 0;
    }
    else
    {
        info->mICACertLen = Encoding::LittleEndian::HostSwap16(mICACertLen);
        memcpy(info->mICACert, mICACert, mICACertLen);
    }

    if (mNOCCert == nullptr || mNOCCertLen == 0)
    {
        info->mNOCCertLen = 0;
    }
    else
    {
        info->mNOCCertLen = Encoding::LittleEndian::HostSwap16(mNOCCertLen);
        memcpy(info->mNOCCert, mNOCCert, mNOCCertLen);
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
    char key[KeySize()];
    ReturnErrorOnFailure(GenerateKey(mFabric, key, sizeof(key)));

    StorableFabricInfo * info = chip::Platform::New<StorableFabricInfo>();
    ReturnErrorCodeIf(info == nullptr, CHIP_ERROR_NO_MEMORY);

    uint16_t infoSize = sizeof(StorableFabricInfo);

    uint16_t id;
    uint16_t rootCertLen, icaCertLen, nocCertLen;
    size_t stringLength;

    SuccessOrExit(err = kvs->SyncGetKeyValue(key, info, infoSize));

    mOperationalId.SetNodeId(Encoding::LittleEndian::HostSwap64(info->mNodeId));
    mOperationalId.SetFabricId(Encoding::LittleEndian::HostSwap64(info->mFabricId));
    id          = Encoding::LittleEndian::HostSwap16(info->mFabric);
    mVendorId   = Encoding::LittleEndian::HostSwap16(info->mVendorId);
    rootCertLen = Encoding::LittleEndian::HostSwap16(info->mRootCertLen);
    icaCertLen  = Encoding::LittleEndian::HostSwap16(info->mICACertLen);
    nocCertLen  = Encoding::LittleEndian::HostSwap16(info->mNOCCertLen);

    stringLength = strnlen(info->mFabricLabel, kFabricLabelMaxLengthInBytes);
    memcpy(mFabricLabel, info->mFabricLabel, stringLength);
    mFabricLabel[stringLength] = '\0'; // Set null terminator

    VerifyOrExit(mFabric == id, err = CHIP_ERROR_INCORRECT_STATE);

    if (mEphemeralKey == nullptr)
    {
#ifdef ENABLE_HSM_CASE_OPS_KEY
        mEphemeralKey = chip::Platform::New<P256KeypairHSM>();
        mEphemeralKey->SetKeyId(CASE_OPS_KEY);
#else
        mEphemeralKey = chip::Platform::New<P256Keypair>();
#endif
    }
    VerifyOrExit(mEphemeralKey != nullptr, err = CHIP_ERROR_NO_MEMORY);
    SuccessOrExit(err = mEphemeralKey->Deserialize(info->mEphemeralKey));

    ChipLogProgress(Inet, "Loading certs from KVS");
    SuccessOrExit(err = SetRootCert(ByteSpan(info->mRootCert, rootCertLen)));
    SuccessOrExit(err = SetICACert(ByteSpan(info->mICACert, icaCertLen)));
    SuccessOrExit(err = SetNOCCert(ByteSpan(info->mNOCCert, nocCertLen)));

exit:
    if (info != nullptr)
    {
        chip::Platform::Delete(info);
    }
    return err;
}

CHIP_ERROR FabricInfo::DeleteFromKVS(PersistentStorageDelegate * kvs, FabricIndex id)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    char key[KeySize()];
    ReturnErrorOnFailure(GenerateKey(id, key, sizeof(key)));

    err = kvs->SyncDeleteKeyValue(key);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Discovery, "Fabric %d is not yet configured", id);
    }
    return err;
}

constexpr size_t FabricInfo::KeySize()
{
    return sizeof(kFabricTableKeyPrefix) + 2 * sizeof(FabricIndex);
}

CHIP_ERROR FabricInfo::GenerateKey(FabricIndex id, char * key, size_t len)
{
    VerifyOrReturnError(len >= KeySize(), CHIP_ERROR_INVALID_ARGUMENT);
    int keySize = snprintf(key, len, "%s%x", kFabricTableKeyPrefix, id);
    VerifyOrReturnError(keySize > 0, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(len > (size_t) keySize, CHIP_ERROR_INTERNAL);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricInfo::SetEphemeralKey(const P256Keypair * key)
{
    P256SerializedKeypair serialized;
    ReturnErrorOnFailure(key->Serialize(serialized));
    if (mEphemeralKey == nullptr)
    {
#ifdef ENABLE_HSM_CASE_OPS_KEY
        mEphemeralKey = chip::Platform::New<P256KeypairHSM>();
        mEphemeralKey->SetKeyId(CASE_OPS_KEY);
#else
        mEphemeralKey = chip::Platform::New<P256Keypair>();
#endif
    }
    VerifyOrReturnError(mEphemeralKey != nullptr, CHIP_ERROR_NO_MEMORY);
    return mEphemeralKey->Deserialize(serialized);
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

    Encoding::LittleEndian::BufferWriter bbuf(mRootPubkey, mRootPubkey.Length());
    bbuf.Put(certData.mPublicKey.data(), certData.mPublicKey.size());
    VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_BUFFER_TOO_SMALL);

    return CHIP_NO_ERROR;
}

void FabricInfo::ReleaseICACert()
{
    if (mICACert != nullptr)
    {
        chip::Platform::MemoryFree(mICACert);
    }
    mICACertLen = 0;
    mICACert    = nullptr;
}

CHIP_ERROR FabricInfo::SetICACert(const ByteSpan & cert)
{
    if (cert.size() == 0)
    {
        ReleaseICACert();
        return CHIP_NO_ERROR;
    }

    VerifyOrReturnError(cert.size() <= kMaxCHIPCertLength, CHIP_ERROR_INVALID_ARGUMENT);
    if (mICACertLen != 0)
    {
        ReleaseICACert();
    }

    VerifyOrReturnError(CanCastTo<uint16_t>(cert.size()), CHIP_ERROR_INVALID_ARGUMENT);
    if (mICACert == nullptr)
    {
        mICACert = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(cert.size()));
    }
    VerifyOrReturnError(mICACert != nullptr, CHIP_ERROR_NO_MEMORY);
    mICACertLen = static_cast<uint16_t>(cert.size());
    memcpy(mICACert, cert.data(), mICACertLen);

    return CHIP_NO_ERROR;
}

void FabricInfo::ReleaseNOCCert()
{
    if (mNOCCert != nullptr)
    {
        chip::Platform::MemoryFree(mNOCCert);
    }
    mNOCCertLen = 0;
    mNOCCert    = nullptr;
}

CHIP_ERROR FabricInfo::SetNOCCert(const ByteSpan & cert)
{
    if (cert.size() == 0)
    {
        ReleaseNOCCert();
        return CHIP_NO_ERROR;
    }

    VerifyOrReturnError(cert.size() <= kMaxCHIPCertLength, CHIP_ERROR_INVALID_ARGUMENT);
    if (mNOCCertLen != 0)
    {
        ReleaseNOCCert();
    }

    ReturnErrorOnFailure(ExtractPeerIdFromOpCert(cert, &mOperationalId));

    MutableByteSpan compressedId(reinterpret_cast<uint8_t *>(&mCompressedFabricId), sizeof(mCompressedFabricId));
    ReturnErrorOnFailure(GenerateCompressedFabricId(mRootPubkey, mOperationalId.GetFabricId(), compressedId));

    VerifyOrReturnError(CanCastTo<uint16_t>(cert.size()), CHIP_ERROR_INVALID_ARGUMENT);
    if (mNOCCert == nullptr)
    {
        mNOCCert = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(cert.size()));
    }
    VerifyOrReturnError(mNOCCert != nullptr, CHIP_ERROR_NO_MEMORY);
    mNOCCertLen = static_cast<uint16_t>(cert.size());
    memcpy(mNOCCert, cert.data(), mNOCCertLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricInfo::SetOperationalCertsFromCertArray(const ByteSpan & certArray)
{
    if (certArray.size() == 0)
    {
        ReleaseNOCCert();
        ReleaseICACert();
        return CHIP_NO_ERROR;
    }

    ByteSpan noc;
    ByteSpan icac;
    ExtractCertsFromCertArray(certArray, noc, icac);

    if (icac.data() != nullptr && icac.size() != 0)
    {
        ReturnErrorOnFailure(SetICACert(icac));
    }

    CHIP_ERROR err = SetNOCCert(noc);
    if (err != CHIP_NO_ERROR)
    {
        ReleaseICACert();
    }

    return err;
}

CHIP_ERROR FabricInfo::VerifyCredentials(const ByteSpan & noc, ValidationContext & context, PeerId & nocPeerId,
                                         Crypto::P256PublicKey & nocPubkey)
{
    ByteSpan icac(mICACert, mICACertLen), rcac(mRootCert, mRootCertLen);

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
    ReturnErrorOnFailure(certificates.FindValidCert(nocSubjectDN, nocSubjectKeyId, context, &resultCert));

    ReturnErrorOnFailure(ExtractPeerIdFromOpCert(certificates.GetCertSet()[nocCertIndex], &nocPeerId));

    if (!icac.empty())
    {
        FabricId icacFabric;
        if (ExtractFabricIdFromCert(certificates.GetCertSet()[1], &icacFabric) == CHIP_NO_ERROR)
        {
            ReturnErrorCodeIf(icacFabric != nocPeerId.GetFabricId(), CHIP_ERROR_INVALID_CASE_PARAMETER);
        }
    }

    Encoding::LittleEndian::BufferWriter bbuf(nocPubkey, nocPubkey.Length());
    bbuf.Put(certificates.GetCertSet()[nocCertIndex].mPublicKey.data(), certificates.GetCertSet()[nocCertIndex].mPublicKey.size());
    VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_BUFFER_TOO_SMALL);

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
    bbuf.Put64(GetFabricId());
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
        if (GenerateDestinationID(ipkList[ipkIdx], initiatorRandom, GetNodeId(), localDestIDSpan) == CHIP_NO_ERROR &&
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

    SetEphemeralKey(newFabric.GetEphemeralKey());
    SetRootCert(ByteSpan(newFabric.mRootCert, newFabric.mRootCertLen));
    SetICACert(ByteSpan(newFabric.mICACert, newFabric.mICACertLen));

    ChipLogProgress(Discovery, "Verifying the received credentials");
    ReturnErrorOnFailure(
        VerifyCredentials(ByteSpan(newFabric.mNOCCert, newFabric.mNOCCertLen), validContext, mOperationalId, pubkey));
    ChipLogProgress(Discovery, "Verifying the received operational ID");
    VerifyOrReturnError(mOperationalId == newFabric.mOperationalId, CHIP_ERROR_INVALID_ARGUMENT);

    SetNOCCert(ByteSpan(newFabric.mNOCCert, newFabric.mNOCCertLen));
    SetVendorId(newFabric.GetVendorId());
    SetFabricLabel(newFabric.GetFabricLabel());
    ChipLogProgress(Discovery, "Added new fabric at index: %d, Initialized: %d", GetFabricIndex(), IsInitialized());
    ChipLogProgress(Discovery, "Assigned fabric ID: 0x" ChipLogFormatX64 ", node ID: 0x" ChipLogFormatX64,
                    ChipLogValueX64(mOperationalId.GetFabricId()), ChipLogValueX64(mOperationalId.GetNodeId()));
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
