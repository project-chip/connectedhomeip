/**
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "ClientMonitoringRegistrationTable.h"

#include <lib/support/DefaultStorageKeyAllocator.h>

namespace chip {

/**********************************************************
 * ClientMonitoringRegistrationTable Implementation
 *********************************************************/

ClientMonitoringRegistrationTable::ClientMonitoringRegistrationTable(PersistentStorageDelegate & storage) : mStorage(storage) {}

CHIP_ERROR ClientMonitoringRegistrationTable::LoadFromStorage(FabricIndex fabricIndex)
{
    uint8_t buffer[kRegStorageSize] = { 0 };
    uint16_t size                   = sizeof(buffer);

    ReturnErrorOnFailure(
        mStorage.SyncGetKeyValue(DefaultStorageKeyAllocator::ClientMonitoringTableEntry(fabricIndex).KeyName(), buffer, size));

    TLV::TLVReader reader;
    reader.Init(buffer, size);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    ReturnErrorOnFailure(mRegisteredClient.Decode(reader));

    mRegisteredClient.fabricIndex = fabricIndex;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClientMonitoringRegistrationTable::SaveToStorage()
{
    VerifyOrReturnError(mRegisteredClient.IsValid(), CHIP_ERROR_INCORRECT_STATE);

    uint8_t buffer[kRegStorageSize] = { 0 };
    TLV::TLVWriter writer;

    writer.Init(buffer);
    ReturnErrorOnFailure(mRegisteredClient.EncodeForWrite(writer, TLV::AnonymousTag()));
    ReturnErrorOnFailure(writer.Finalize());

    return mStorage.SyncSetKeyValue(DefaultStorageKeyAllocator::ClientMonitoringTableEntry(mRegisteredClient.fabricIndex).KeyName(),
                                    buffer, static_cast<uint16_t>(writer.GetLengthWritten()));
}

ClientMonitoringRegistrationTable::ClientRegistrationEntry & ClientMonitoringRegistrationTable::GetClientRegistrationEntry()
{
    return mRegisteredClient;
}

CHIP_ERROR ClientMonitoringRegistrationTable::DeleteFromStorage(FabricIndex fabric)
{
    return mStorage.SyncDeleteKeyValue(DefaultStorageKeyAllocator::ClientMonitoringTableEntry(fabric).KeyName());
}

bool ClientMonitoringRegistrationTable::HasValueForFabric(FabricIndex fabric)
{
    return mStorage.SyncDoesKeyExist(DefaultStorageKeyAllocator::ClientMonitoringTableEntry(fabric).KeyName());
}

} // namespace chip
