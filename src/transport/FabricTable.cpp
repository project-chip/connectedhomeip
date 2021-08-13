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
#include <support/CHIPMemString.h>
#include <support/SafeInt.h>
#include <transport/FabricTable.h>
#if CHIP_CRYPTO_HSM
#include <crypto/hsm/CHIPCryptoPALHsm.h>
#endif

namespace chip {
using namespace Credentials;
using namespace Crypto;

namespace Transport {

CHIP_ERROR FabricInfo::SetFabricLabel(chip::ByteSpan label)
{
    Platform::CopyString(mFabricLabel, sizeof(mFabricLabel), label);
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
        ChipLogError(Discovery, "Error occurred calling SyncDeleteKeyValue");
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
    mRootCertLen          = static_cast<uint16_t>(cert.size());
    mRootCertAllocatedLen = (mRootCertLen > mRootCertAllocatedLen) ? mRootCertLen : mRootCertAllocatedLen;
    memcpy(mRootCert, cert.data(), mRootCertLen);

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

CHIP_ERROR FabricInfo::GetCredentials(OperationalCredentialSet & credentials, ChipCertificateSet & certificates,
                                      CertificateKeyId & rootKeyId, uint8_t & credentialsIndex)
{
    constexpr uint8_t kMaxNumCertsInOpCreds = 3;
    ReturnErrorOnFailure(certificates.Init(kMaxNumCertsInOpCreds));

    ReturnErrorOnFailure(
        certificates.LoadCert(mRootCert, mRootCertLen,
                              BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor).Set(CertDecodeFlags::kGenerateTBSHash)));

    if (mICACert != nullptr && mICACertLen > 0)
    {
        ReturnErrorOnFailure(
            certificates.LoadCert(mICACert, mICACertLen, BitFlags<CertDecodeFlags>(CertDecodeFlags::kGenerateTBSHash)));
    }

    credentials.Release();
    ReturnErrorOnFailure(credentials.Init(&certificates, 1));
    credentialsIndex = static_cast<uint8_t>(credentials.GetCertCount() - 1U);

    rootKeyId = credentials.GetTrustedRootId(0);

    ReturnErrorOnFailure(credentials.SetDevOpCred(rootKeyId, mNOCCert, mNOCCertLen));
    ReturnErrorOnFailure(credentials.SetDevOpCredKeypair(rootKeyId, mEphemeralKey));

    return CHIP_NO_ERROR;
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
        return &mStates[fabricIndex - kMinValidFabricIndex];
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

CHIP_ERROR FabricTable::LoadFromStorage(FabricIndex id)
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    FabricInfo * fabric  = nullptr;
    bool didCreateFabric = false;
    VerifyOrExit(mStorage != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    fabric = FindFabricWithIndex(id);
    VerifyOrExit(fabric != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    if (!fabric->IsInitialized())
    {
        didCreateFabric = true;

        err = fabric->FetchFromKVS(mStorage);
        SuccessOrExit(err);
    }

exit:
    if (err != CHIP_NO_ERROR && didCreateFabric)
    {
        ReleaseFabricIndex(id);
    }
    else if (err == CHIP_NO_ERROR && mDelegate != nullptr)
    {
        ChipLogProgress(Discovery, "Fabric (%d) loaded from storage. Calling OnFabricRetrievedFromStorage", id);
        mDelegate->OnFabricRetrievedFromStorage(fabric);
    }
    return err;
}

CHIP_ERROR FabricTable::SetFabricInfoIfIndexAvailable(FabricIndex index, FabricInfo & newFabric)
{
    FabricInfo * fabric = FindFabricWithIndex(index);

    if (fabric != nullptr && !fabric->IsInitialized())
    {
        fabric->SetEphemeralKey(newFabric.GetEphemeralKey());
        fabric->SetRootCert(ByteSpan(newFabric.mRootCert, newFabric.mRootCertLen));
        fabric->SetICACert(ByteSpan(newFabric.mICACert, newFabric.mICACertLen));
        fabric->SetNOCCert(ByteSpan(newFabric.mNOCCert, newFabric.mNOCCertLen));
        fabric->SetOperationalId(newFabric.mOperationalId);
        fabric->SetVendorId(newFabric.GetVendorId());
        fabric->SetFabricLabel(chip::ByteSpan(
            newFabric.GetFabricLabel(), strnlen(Uint8::to_const_char(newFabric.GetFabricLabel()), kFabricLabelMaxLengthInBytes)));
        ChipLogProgress(Discovery, "Added new fabric at index: %d, Initialized: %d", fabric->GetFabricIndex(),
                        fabric->IsInitialized());
        ChipLogProgress(Discovery, "Assigned fabric ID: 0x" ChipLogFormatX64 ", node ID: 0x" ChipLogFormatX64,
                        ChipLogValueX64(fabric->mOperationalId.GetFabricId()), ChipLogValueX64(fabric->mOperationalId.GetNodeId()));
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_INVALID_ARGUMENT;
}

CHIP_ERROR FabricTable::AddNewFabric(FabricInfo & newFabric, FabricIndex * outputIndex)
{
    VerifyOrReturnError(outputIndex != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    static_assert(kMaxValidFabricIndex <= UINT8_MAX, "Cannot create more fabrics than UINT8_MAX");
    for (FabricIndex i = mNextAvailableFabricIndex; i <= kMaxValidFabricIndex; i++)
    {
        if (CHIP_NO_ERROR == SetFabricInfoIfIndexAvailable(i, newFabric))
        {
            mNextAvailableFabricIndex = static_cast<FabricIndex>((i + 1) % UINT8_MAX);
            *outputIndex              = i;
            return CHIP_NO_ERROR;
        }
    }

    for (FabricIndex i = kMinValidFabricIndex; i < kMaxValidFabricIndex; i++)
    {
        if (CHIP_NO_ERROR == SetFabricInfoIfIndexAvailable(i, newFabric))
        {
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
