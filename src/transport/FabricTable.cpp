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

    info->mNodeId   = Encoding::LittleEndian::HostSwap64(mNodeId);
    info->mFabric   = Encoding::LittleEndian::HostSwap16(mFabric);
    info->mFabricId = Encoding::LittleEndian::HostSwap64(mFabricId);
    info->mVendorId = Encoding::LittleEndian::HostSwap16(mVendorId);

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

    mNodeId     = Encoding::LittleEndian::HostSwap64(info->mNodeId);
    id          = Encoding::LittleEndian::HostSwap16(info->mFabric);
    mFabricId   = Encoding::LittleEndian::HostSwap64(info->mFabricId);
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

    ChipLogProgress(Inet, "Loading certs from KVS");
    SuccessOrExit(SetRootCert(ByteSpan(info->mRootCert, rootCertLen)));
    SuccessOrExit(SetICACert(ByteSpan(info->mICACert, icaCertLen)));
    SuccessOrExit(SetNOCCert(ByteSpan(info->mNOCCert, nocCertLen)));

exit:
    if (info != nullptr)
    {
        chip::Platform::Delete(info);
    }
    return CHIP_NO_ERROR;
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

CHIP_ERROR FabricInfo::SetOperationalKey(const P256Keypair & key)
{
    P256SerializedKeypair serialized;
    ReturnErrorOnFailure(key.Serialize(serialized));
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

#if 0
<<<<<<< HEAD:src/transport/AdminPairingTable.cpp
void AdminPairingInfo::ReleaseDeviceDAC()
{
    if (mDeviceDAC != nullptr)
    {
        chip::Platform::MemoryFree(mDeviceDAC);
    }
    mDeviceDACLen = 0;
    mDeviceDAC    = nullptr;
}

CHIP_ERROR AdminPairingInfo::SetDeviceDAC(const chip::ByteSpan & dac)
{
    if (dac.size() == 0)
    {
        ReleaseDeviceDAC();
        return CHIP_NO_ERROR;
    }

    VerifyOrReturnError(dac.size() <= Crypto::kMax_x509_Certificate_Length, CHIP_ERROR_INVALID_ARGUMENT);
    if (mDeviceDACLen != 0)
    {
        ReleaseDeviceDAC();
    }

    VerifyOrReturnError(CanCastTo<uint16_t>(dac.size()), CHIP_ERROR_INVALID_ARGUMENT);
    if (mDeviceDAC == nullptr)
    {
        mDeviceDAC = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(dac.size()));
    }
    VerifyOrReturnError(mDeviceDAC != nullptr, CHIP_ERROR_NO_MEMORY);
    mDeviceDACLen = static_cast<uint16_t>(dac.size());
    memcpy(mDeviceDAC, dac.data(), mDeviceDACLen);

    return CHIP_NO_ERROR;
}

void AdminPairingInfo::ReleaseDevicePAI()
{
    if (mDevicePAI != nullptr)
    {
        chip::Platform::MemoryFree(mDevicePAI);
    }
    mDevicePAILen = 0;
    mDevicePAI    = nullptr;
}

CHIP_ERROR AdminPairingInfo::SetDevicePAI(const chip::ByteSpan & pai)
{
    if (pai.size() == 0)
    {
        ReleaseDevicePAI();
        return CHIP_NO_ERROR;
    }

    VerifyOrReturnError(pai.size() <= Crypto::kMax_x509_Certificate_Length, CHIP_ERROR_INVALID_ARGUMENT);
    if (mDevicePAILen != 0)
    {
        ReleaseDevicePAI();
    }

    VerifyOrReturnError(CanCastTo<uint16_t>(pai.size()), CHIP_ERROR_INVALID_ARGUMENT);
    if (mDevicePAI == nullptr)
    {
        mDevicePAI = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(pai.size()));
    }
    VerifyOrReturnError(mDevicePAI != nullptr, CHIP_ERROR_NO_MEMORY);
    mDevicePAILen = static_cast<uint16_t>(pai.size());
    memcpy(mDevicePAI, pai.data(), mDevicePAILen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR AdminPairingInfo::GetCredentials(OperationalCredentialSet & credentials, ChipCertificateSet & certificates,
                                            CertificateKeyId & rootKeyId)
#endif
CHIP_ERROR FabricInfo::GetCredentials(OperationalCredentialSet & credentials, ChipCertificateSet & certificates,
                                      CertificateKeyId & rootKeyId)
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

    rootKeyId = credentials.GetTrustedRootId(0);

    ReturnErrorOnFailure(credentials.SetDevOpCred(rootKeyId, mNOCCert, mNOCCertLen));
    ReturnErrorOnFailure(credentials.SetDevOpCredKeypair(rootKeyId, mOperationalKey));

    return CHIP_NO_ERROR;
}

FabricInfo * FabricTable::AssignFabricIndex(FabricIndex fabricIndex)
{
    for (size_t i = 0; i < CHIP_CONFIG_MAX_DEVICE_ADMINS; i++)
    {
        if (!mStates[i].IsInitialized())
        {
            mStates[i].SetFabricIndex(fabricIndex);

            return &mStates[i];
        }
    }

    return nullptr;
}

FabricInfo * FabricTable::AssignFabricIndex(FabricIndex fabricIndex, NodeId nodeId)
{
    FabricInfo * fabric = AssignFabricIndex(fabricIndex);

    if (fabric != nullptr)
    {
        fabric->SetNodeId(nodeId);
    }

    return fabric;
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
    for (auto & state : mStates)
    {
        if (state.IsInitialized() && state.GetFabricIndex() == fabricIndex)
        {
            return &state;
        }
    }

    return nullptr;
}

FabricInfo * FabricTable::FindFabricForNode(FabricId fabricId, NodeId nodeId, uint16_t vendorId)
{
    uint32_t index = 0;
    for (auto & state : mStates)
    {
        if (state.IsInitialized())
        {
            ChipLogProgress(Discovery,
                            "Checking ind:%" PRIu32 " [fabricId 0x" ChipLogFormatX64 " nodeId 0x" ChipLogFormatX64
                            " vendorId %" PRIu16 "] vs"
                            " [fabricId 0x" ChipLogFormatX64 " nodeId 0x" ChipLogFormatX64 " vendorId %d]",
                            index, ChipLogValueX64(state.GetFabricId()), ChipLogValueX64(state.GetNodeId()), state.GetVendorId(),
                            ChipLogValueX64(fabricId), ChipLogValueX64(nodeId), vendorId);
        }
        if (state.IsInitialized() && state.GetFabricId() == fabricId &&
            (nodeId == kUndefinedNodeId || state.GetNodeId() == nodeId) &&
            (vendorId == kUndefinedVendorId || state.GetVendorId() == vendorId))
        {
            ChipLogProgress(Discovery, "Found a match!");
            return &state;
        }
        index++;
    }

    return nullptr;
}

void FabricTable::Reset()
{
    for (size_t i = 0; i < CHIP_CONFIG_MAX_DEVICE_ADMINS; i++)
    {
        mStates[i].Reset();
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
    if (fabric == nullptr)
    {
        fabric          = AssignFabricIndex(id);
        didCreateFabric = true;
    }
    VerifyOrExit(fabric != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    err = fabric->FetchFromKVS(mStorage);

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
