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

// TODO: Admin Pairing table should be backed by a single backing store (attribute store), remove delegate callbacks.
namespace {
PersistentStorageDelegate * gStorage             = nullptr;
Transport::AdminPairingTableDelegate * gDelegate = nullptr;
} // anonymous namespace

namespace Transport {

CHIP_ERROR AdminPairingInfo::StoreIntoKVS()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (gStorage == nullptr)
    {
        ChipLogError(Discovery, "Server storage delegate is null, cannot store admin in KVS");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    char key[KeySize()];
    ReturnErrorOnFailure(GenerateKey(mAdmin, key, sizeof(key)));

    StorableAdminPairingInfo info;
    info.mNodeId   = Encoding::LittleEndian::HostSwap64(mNodeId);
    info.mAdmin    = Encoding::LittleEndian::HostSwap16(mAdmin);
    info.mFabricId = Encoding::LittleEndian::HostSwap64(mFabricId);
    info.mVendorId = Encoding::LittleEndian::HostSwap16(mVendorId);

    err = gStorage->SyncSetKeyValue(key, &info, sizeof(info));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Error occurred calling SyncSetKeyValue.");
    }
    else if (gDelegate != nullptr)
    {
        ChipLogProgress(Discovery, "New admin (%d)  to KVS store, calling OnAdminPersistedToStorage.", info.mAdmin);
        gDelegate->OnAdminPersistedToStorage(mAdmin, mFabricId, mNodeId);
    }

    return err;
}

CHIP_ERROR AdminPairingInfo::FetchFromKVS()
{
    if (gStorage == nullptr)
    {
        ChipLogError(Discovery, "Server storage delegate is null, cannot fetch from KVS");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    char key[KeySize()];
    ReturnErrorOnFailure(GenerateKey(mAdmin, key, sizeof(key)));

    StorableAdminPairingInfo info;

    uint16_t size = sizeof(info);
    ReturnErrorOnFailure(gStorage->SyncGetKeyValue(key, &info, size));

    mNodeId    = Encoding::LittleEndian::HostSwap64(info.mNodeId);
    AdminId id = Encoding::LittleEndian::HostSwap16(info.mAdmin);
    mFabricId  = Encoding::LittleEndian::HostSwap64(info.mFabricId);
    mVendorId  = Encoding::LittleEndian::HostSwap16(info.mVendorId);
    ReturnErrorCodeIf(mAdmin != id, CHIP_ERROR_INCORRECT_STATE);

    if (gDelegate != nullptr)
    {
        ChipLogProgress(Discovery, "New admin (%d) fetched from KVS store. Calling OnAdminRetrievedFromStorage.", info.mAdmin);
        gDelegate->OnAdminRetrievedFromStorage(id, mFabricId, mNodeId);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AdminPairingInfo::DeleteFromKVS(AdminId id)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (gStorage == nullptr)
    {
        ChipLogError(Discovery, "Server storage delegate is null, cannot delete from KVS");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    char key[KeySize()];
    ReturnErrorOnFailure(GenerateKey(id, key, sizeof(key)));

    err = gStorage->SyncDeleteKeyValue(key);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Error occurred calling SyncDeleteKeyValue");
    }
    else if (gDelegate != nullptr)
    {
        ChipLogProgress(Discovery, "Admin (%d) removed from  from KVS store, calling OnAdminDeletedFromStorage", id);
        gDelegate->OnAdminDeletedFromStorage(id);
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
    AdminPairingInfo * admin = FindAdmin(adminId);
    if (admin != nullptr)
    {
        admin->Reset();
    }
}

AdminPairingInfo * AdminPairingTable::FindAdmin(AdminId adminId)
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

AdminPairingInfo * AdminPairingTable::FindAdmin(FabricId fabricId)
{
    uint32_t index = 0;
    for (auto & state : mStates)
    {
        if (state.IsInitialized())
        {
            ChipLogProgress(Discovery, "Looking at index %d with fabricID %llu nodeID %llu to see if it matches fabricId %llu.",
                            index, state.GetFabricId(), state.GetNodeId(), fabricId);
        }
        if (state.IsInitialized() && state.GetFabricId() == fabricId)
        {
            ChipLogProgress(Discovery, "Found a match!");
            return &state;
        }
        index++;
    }

    return nullptr;
}

AdminPairingInfo * AdminPairingTable::FindAdmin(FabricId fabricId, NodeId nodeId)
{
    uint32_t index = 0;
    for (auto & state : mStates)
    {
        if (state.IsInitialized())
        {
            ChipLogProgress(Discovery,
                            "Looking at index %d with fabricID %llu nodeID %llu to see if it matches fabricId %llu nodeId %llu.",
                            index, state.GetFabricId(), state.GetNodeId(), fabricId, nodeId);
        }
        if (state.IsInitialized() && state.GetFabricId() == fabricId && state.GetNodeId() == nodeId)
        {
            ChipLogProgress(Discovery, "Found a match!");
            return &state;
        }
        index++;
    }

    return nullptr;
}

AdminPairingInfo * AdminPairingTable::FindAdmin(FabricId fabricId, NodeId nodeId, uint16_t vendorId)
{
    uint32_t index = 0;
    for (auto & state : mStates)
    {
        if (state.IsInitialized())
        {
            ChipLogProgress(Discovery,
                            "Looking at index %d with fabricID %llu nodeID %llu vendorId %d to see if it matches fabricId %llu "
                            "nodeId %llu vendorId %d.",
                            index, state.GetFabricId(), state.GetNodeId(), state.GetVendorId(), fabricId, nodeId, vendorId);
        }
        if (state.IsInitialized() && state.GetFabricId() == fabricId && state.GetNodeId() == nodeId &&
            state.GetVendorId() == vendorId)
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

CHIP_ERROR AdminPairingTable::Init(PersistentStorageDelegate * storage)
{
    VerifyOrReturnError(storage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    gStorage = storage;
    ChipLogProgress(Discovery, "Init admin pairing table with server storage.");
    return CHIP_NO_ERROR;
}

CHIP_ERROR AdminPairingTable::SetAdminPairingDelegate(AdminPairingTableDelegate * delegate)
{
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    gDelegate = delegate;
    ChipLogProgress(Discovery, "Set the admin pairing table delegate");
    return CHIP_NO_ERROR;
}

} // namespace Transport
} // namespace chip
