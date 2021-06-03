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
 * @brief Defines a table of admins that have provisioned the device.
 */

#include <core/CHIPEncoding.h>
#include <support/CHIPMem.h>
#include <support/SafeInt.h>
#include <transport/AdminPairingTable.h>

namespace chip {
using namespace Credentials;
using namespace Crypto;

namespace Transport {

CHIP_ERROR AdminPairingInfo::SetFabricLabel(const uint8_t * fabricLabel)
{
    const char * charFabricLabel = Uint8::to_const_char(fabricLabel);
    size_t stringLength          = strnlen(charFabricLabel, kFabricLabelMaxLengthInBytes);
    memcpy(mFabricLabel, charFabricLabel, stringLength);
    mFabricLabel[stringLength] = '\0'; // Set null terminator

    return CHIP_NO_ERROR;
}

CHIP_ERROR AdminPairingInfo::StoreIntoKVS(PersistentStorageDelegate * kvs)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    char key[KeySize()];
    ReturnErrorOnFailure(GenerateKey(mAdmin, key, sizeof(key)));

    StorableAdminPairingInfo * info = chip::Platform::New<StorableAdminPairingInfo>();
    ReturnErrorCodeIf(info == nullptr, CHIP_ERROR_NO_MEMORY);

    info->mNodeId   = Encoding::LittleEndian::HostSwap64(mNodeId);
    info->mAdmin    = Encoding::LittleEndian::HostSwap16(mAdmin);
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

    if (mOperationalCert == nullptr || mOpCertLen == 0)
    {
        info->mOpCertLen = 0;
    }
    else
    {
        info->mOpCertLen = Encoding::LittleEndian::HostSwap16(mOpCertLen);
        memcpy(info->mOperationalCert, mOperationalCert, mOpCertLen);
    }

    err = kvs->SyncSetKeyValue(key, info, sizeof(StorableAdminPairingInfo));
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

CHIP_ERROR AdminPairingInfo::FetchFromKVS(PersistentStorageDelegate * kvs)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    char key[KeySize()];
    ReturnErrorOnFailure(GenerateKey(mAdmin, key, sizeof(key)));

    StorableAdminPairingInfo * info = chip::Platform::New<StorableAdminPairingInfo>();
    ReturnErrorCodeIf(info == nullptr, CHIP_ERROR_NO_MEMORY);

    uint16_t infoSize = sizeof(StorableAdminPairingInfo);

    AdminId id;
    uint16_t rootCertLen, opCertLen;
    size_t stringLength;

    SuccessOrExit(err = kvs->SyncGetKeyValue(key, info, infoSize));

    mNodeId     = Encoding::LittleEndian::HostSwap64(info->mNodeId);
    id          = Encoding::LittleEndian::HostSwap16(info->mAdmin);
    mFabricId   = Encoding::LittleEndian::HostSwap64(info->mFabricId);
    mVendorId   = Encoding::LittleEndian::HostSwap16(info->mVendorId);
    rootCertLen = Encoding::LittleEndian::HostSwap16(info->mRootCertLen);
    opCertLen   = Encoding::LittleEndian::HostSwap16(info->mOpCertLen);

    stringLength = strnlen(info->mFabricLabel, kFabricLabelMaxLengthInBytes);
    memcpy(mFabricLabel, info->mFabricLabel, stringLength);
    mFabricLabel[stringLength] = '\0'; // Set null terminator

    VerifyOrExit(mAdmin == id, err = CHIP_ERROR_INCORRECT_STATE);

    if (mOperationalKey == nullptr)
    {
        mOperationalKey = chip::Platform::New<P256Keypair>();
    }
    VerifyOrExit(mOperationalKey != nullptr, err = CHIP_ERROR_NO_MEMORY);
    SuccessOrExit(err = mOperationalKey->Deserialize(info->mOperationalKey));

    ChipLogProgress(Inet, "Loading certs from KVS");
    SuccessOrExit(SetRootCert(ByteSpan(info->mRootCert, rootCertLen)));
    SuccessOrExit(SetOperationalCert(ByteSpan(info->mOperationalCert, opCertLen)));

exit:
    if (info != nullptr)
    {
        chip::Platform::Delete(info);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR AdminPairingInfo::DeleteFromKVS(PersistentStorageDelegate * kvs, AdminId id)
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

constexpr size_t AdminPairingInfo::KeySize()
{
    return sizeof(kAdminTableKeyPrefix) + 2 * sizeof(AdminId);
}

CHIP_ERROR AdminPairingInfo::GenerateKey(AdminId id, char * key, size_t len)
{
    VerifyOrReturnError(len >= KeySize(), CHIP_ERROR_INVALID_ARGUMENT);
    int keySize = snprintf(key, len, "%s%x", kAdminTableKeyPrefix, id);
    VerifyOrReturnError(keySize > 0, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(len > (size_t) keySize, CHIP_ERROR_INTERNAL);
    return CHIP_NO_ERROR;
}

CHIP_ERROR AdminPairingInfo::SetOperationalKey(const P256Keypair & key)
{
    P256SerializedKeypair serialized;
    ReturnErrorOnFailure(key.Serialize(serialized));
    if (mOperationalKey == nullptr)
    {
        mOperationalKey = chip::Platform::New<P256Keypair>();
    }
    VerifyOrReturnError(mOperationalKey != nullptr, CHIP_ERROR_NO_MEMORY);
    return mOperationalKey->Deserialize(serialized);
}

void AdminPairingInfo::ReleaseRootCert()
{
    if (mRootCert != nullptr)
    {
        chip::Platform::MemoryFree(mRootCert);
    }
    mRootCertAllocatedLen = 0;
    mRootCertLen          = 0;
    mRootCert             = nullptr;
}

CHIP_ERROR AdminPairingInfo::SetRootCert(const ByteSpan & cert)
{
    if (cert.size() == 0)
    {
        ReleaseRootCert();
        return CHIP_NO_ERROR;
    }

    VerifyOrReturnError(cert.size() <= kMaxChipCertSize, CHIP_ERROR_INVALID_ARGUMENT);
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

void AdminPairingInfo::ReleaseOperationalCert()
{
    if (mOperationalCert != nullptr)
    {
        chip::Platform::MemoryFree(mOperationalCert);
    }
    mOpCertAllocatedLen = 0;
    mOpCertLen          = 0;
    mOperationalCert    = nullptr;
}

CHIP_ERROR AdminPairingInfo::SetOperationalCert(const ByteSpan & cert)
{
    if (cert.size() == 0)
    {
        ReleaseOperationalCert();
        return CHIP_NO_ERROR;
    }

    // There could be two certs in the set -> ICA and NOC
    VerifyOrReturnError(cert.size() <= kMaxChipCertSize * 2, CHIP_ERROR_INVALID_ARGUMENT);
    if (mOpCertLen != 0 && mOpCertAllocatedLen < cert.size())
    {
        ReleaseOperationalCert();
    }

    if (mOperationalCert == nullptr)
    {
        mOperationalCert = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(cert.size()));
    }
    VerifyOrReturnError(mOperationalCert != nullptr, CHIP_ERROR_NO_MEMORY);
    VerifyOrReturnError(CanCastTo<uint16_t>(cert.size()), CHIP_ERROR_INVALID_ARGUMENT);
    mOpCertLen          = static_cast<uint16_t>(cert.size());
    mOpCertAllocatedLen = (mOpCertLen > mOpCertAllocatedLen) ? mOpCertLen : mOpCertAllocatedLen;
    memcpy(mOperationalCert, cert.data(), mOpCertLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR AdminPairingInfo::GetCredentials(OperationalCredentialSet & credentials, ChipCertificateSet & certificates,
                                            CertificateKeyId & rootKeyId)
{
    constexpr uint8_t kMaxNumCertsInOpCreds = 3;
    ReturnErrorOnFailure(certificates.Init(kMaxNumCertsInOpCreds, kMaxChipCertSize * kMaxNumCertsInOpCreds));

    ReturnErrorOnFailure(
        certificates.LoadCert(mRootCert, mRootCertLen,
                              BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor).Set(CertDecodeFlags::kGenerateTBSHash)));

    credentials.Release();
    ReturnErrorOnFailure(credentials.Init(&certificates, certificates.GetCertCount()));

    const CertificateKeyId * id = credentials.GetTrustedRootId(0);
    rootKeyId.mId               = id->mId;
    rootKeyId.mLen              = id->mLen;

    ReturnErrorOnFailure(credentials.SetDevOpCred(rootKeyId, mOperationalCert, mOpCertLen));
    ReturnErrorOnFailure(credentials.SetDevOpCredKeypair(rootKeyId, mOperationalKey));

    return CHIP_NO_ERROR;
}

AdminPairingInfo * AdminPairingTable::AssignAdminId(AdminId adminId)
{
    for (size_t i = 0; i < CHIP_CONFIG_MAX_DEVICE_ADMINS; i++)
    {
        if (!mStates[i].IsInitialized())
        {
            mStates[i].SetAdminId(adminId);

            return &mStates[i];
        }
    }

    return nullptr;
}

AdminPairingInfo * AdminPairingTable::AssignAdminId(AdminId adminId, NodeId nodeId)
{
    AdminPairingInfo * admin = AssignAdminId(adminId);

    if (admin != nullptr)
    {
        admin->SetNodeId(nodeId);
    }

    return admin;
}

void AdminPairingTable::ReleaseAdminId(AdminId adminId)
{
    AdminPairingInfo * admin = FindAdminWithId(adminId);
    if (admin != nullptr)
    {
        admin->Reset();
    }
}

AdminPairingInfo * AdminPairingTable::FindAdminWithId(AdminId adminId)
{
    for (auto & state : mStates)
    {
        if (state.IsInitialized() && state.GetAdminId() == adminId)
        {
            return &state;
        }
    }

    return nullptr;
}

AdminPairingInfo * AdminPairingTable::FindAdminForNode(FabricId fabricId, NodeId nodeId, uint16_t vendorId)
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

void AdminPairingTable::Reset()
{
    for (size_t i = 0; i < CHIP_CONFIG_MAX_DEVICE_ADMINS; i++)
    {
        mStates[i].Reset();
    }
}

CHIP_ERROR AdminPairingTable::Store(AdminId id)
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    AdminPairingInfo * admin = nullptr;

    VerifyOrExit(mStorage != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    admin = FindAdminWithId(id);
    VerifyOrExit(admin != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    err = admin->StoreIntoKVS(mStorage);
exit:
    if (err == CHIP_NO_ERROR && mDelegate != nullptr)
    {
        ChipLogProgress(Discovery, "Admin (%d) persisted to storage. Calling OnAdminPersistedToStorage", id);
        mDelegate->OnAdminPersistedToStorage(admin);
    }
    return err;
}

CHIP_ERROR AdminPairingTable::LoadFromStorage(AdminId id)
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    AdminPairingInfo * admin = nullptr;
    bool didCreateAdmin      = false;
    VerifyOrExit(mStorage != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    admin = FindAdminWithId(id);
    if (admin == nullptr)
    {
        admin          = AssignAdminId(id);
        didCreateAdmin = true;
    }
    VerifyOrExit(admin != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    err = admin->FetchFromKVS(mStorage);

exit:
    if (err != CHIP_NO_ERROR && didCreateAdmin)
    {
        ReleaseAdminId(id);
    }
    else if (err == CHIP_NO_ERROR && mDelegate != nullptr)
    {
        ChipLogProgress(Discovery, "Admin (%d) loaded from storage. Calling OnAdminRetrievedFromStorage", id);
        mDelegate->OnAdminRetrievedFromStorage(admin);
    }
    return err;
}

CHIP_ERROR AdminPairingTable::Delete(AdminId id)
{
    AdminPairingInfo * admin = nullptr;
    CHIP_ERROR err           = CHIP_NO_ERROR;
    bool adminIsInitialized  = false;
    VerifyOrExit(mStorage != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    admin              = FindAdminWithId(id);
    adminIsInitialized = admin != nullptr && admin->IsInitialized();
    err                = AdminPairingInfo::DeleteFromKVS(mStorage, id); // Delete from storage regardless

exit:
    if (err == CHIP_NO_ERROR)
    {
        ReleaseAdminId(id);
        if (mDelegate != nullptr && adminIsInitialized)
        {
            ChipLogProgress(Discovery, "Admin (%d) deleted. Calling OnAdminDeletedFromStorage", id);
            mDelegate->OnAdminDeletedFromStorage(id);
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR AdminPairingTable::Init(PersistentStorageDelegate * storage)
{
    VerifyOrReturnError(storage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mStorage = storage;
    ChipLogDetail(Discovery, "Init admin pairing table with server storage");
    return CHIP_NO_ERROR;
}

CHIP_ERROR AdminPairingTable::SetAdminPairingDelegate(AdminPairingTableDelegate * delegate)
{
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mDelegate = delegate;
    ChipLogDetail(Discovery, "Set the admin pairing table delegate");
    return CHIP_NO_ERROR;
}

} // namespace Transport
} // namespace chip
