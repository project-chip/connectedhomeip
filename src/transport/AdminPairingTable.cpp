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
#include <support/ReturnMacros.h>
#include <support/SafeInt.h>
#include <transport/AdminPairingTable.h>

namespace chip {
namespace Transport {

CHIP_ERROR AdminPairingInfo::StoreIntoKVS(PersistentStorageDelegate & kvs)
{
    char key[KeySize()];
    ReturnErrorOnFailure(GenerateKey(mAdmin, key, sizeof(key)));

    StorableAdminPairingInfo info;
    info.mNodeId = Encoding::LittleEndian::HostSwap64(mNodeId);
    info.mAdmin  = Encoding::LittleEndian::HostSwap16(mAdmin);

    return kvs.SetKeyValue(key, &info, sizeof(info));
}

CHIP_ERROR AdminPairingInfo::FetchFromKVS(PersistentStorageDelegate & kvs)
{
    char key[KeySize()];
    ReturnErrorOnFailure(GenerateKey(mAdmin, key, sizeof(key)));

    StorableAdminPairingInfo info;

    uint16_t size = sizeof(info);
    ReturnErrorOnFailure(kvs.GetKeyValue(key, &info, size));

    mNodeId    = Encoding::LittleEndian::HostSwap64(info.mNodeId);
    AdminId id = Encoding::LittleEndian::HostSwap16(info.mAdmin);
    ReturnErrorCodeIf(mAdmin != id, CHIP_ERROR_INCORRECT_STATE);

    return CHIP_NO_ERROR;
}

CHIP_ERROR AdminPairingInfo::DeleteFromKVS(PersistentStorageDelegate & kvs, AdminId id)
{
    char key[KeySize()];
    ReturnErrorOnFailure(GenerateKey(id, key, sizeof(key)));

    kvs.DeleteKeyValue(key);
    return CHIP_NO_ERROR;
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
    for (size_t i = 0; i < CHIP_CONFIG_MAX_DEVICE_ADMINS; i++)
    {
        if (mStates[i].IsInitialized() && mStates[i].GetAdminId() == adminId)
        {
            return &mStates[i];
        }
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

} // namespace Transport
} // namespace chip
