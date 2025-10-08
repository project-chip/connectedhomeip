/*
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
#pragma once

#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/TLV.h>
#include <lib/support/DefaultStorageKeyAllocator.h>

namespace chip {
/// @brief Data accessor allowing data to be persisted by PersistentStore to be accessed
struct DataAccessor
{
    virtual ~DataAccessor()                                     = default;
    virtual CHIP_ERROR UpdateKey(StorageKeyName & key) const    = 0;
    virtual CHIP_ERROR Serialize(TLV::TLVWriter & writer) const = 0;
    virtual CHIP_ERROR Deserialize(TLV::TLVReader & reader)     = 0;
    virtual void Clear()                                        = 0;
};

/// @brief Interface to PersistentStorageDelegate allowing storage of data of variable size such as TLV, delegating data access
/// to DataAccessor
/// @tparam kMaxSerializedSize size of the mBuffer necessary to retrieve an entry from the storage. Varies with the type of data
/// stored. Will be allocated on the stack so the implementation needs to be aware of this when choosing this value.
template <size_t kMaxSerializedSize>
struct PersistentStore
{
    virtual ~PersistentStore() = default;

    CHIP_ERROR Save(const DataAccessor & persistent, PersistentStorageDelegate * storage)
    {
        VerifyOrReturnError(nullptr != storage, CHIP_ERROR_INVALID_ARGUMENT);

        StorageKeyName key = StorageKeyName::Uninitialized();
        ReturnErrorOnFailure(persistent.UpdateKey(key));

        // Serialize the data
        TLV::TLVWriter writer;
        writer.Init(mBuffer, sizeof(mBuffer));

        ReturnErrorOnFailure(persistent.Serialize(writer));

        // Save serialized data
        return storage->SyncSetKeyValue(key.KeyName(), mBuffer, static_cast<uint16_t>(writer.GetLengthWritten()));
    }

    CHIP_ERROR Load(DataAccessor & persistent, PersistentStorageDelegate * storage)
    {
        VerifyOrReturnError(nullptr != storage, CHIP_ERROR_INVALID_ARGUMENT);

        StorageKeyName key = StorageKeyName::Uninitialized();

        // Update storage key
        ReturnErrorOnFailure(persistent.UpdateKey(key));

        // Set data to defaults
        persistent.Clear();

        // Load the serialized data
        uint16_t size  = static_cast<uint16_t>(sizeof(mBuffer));
        CHIP_ERROR err = storage->SyncGetKeyValue(key.KeyName(), mBuffer, size);
        VerifyOrReturnError(CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND != err, CHIP_ERROR_NOT_FOUND);
        ReturnErrorOnFailure(err);

        // Decode serialized data
        TLV::TLVReader reader;
        reader.Init(mBuffer, size);
        return persistent.Deserialize(reader);
    }

    CHIP_ERROR Delete(DataAccessor & persistent, PersistentStorageDelegate * storage)
    {
        VerifyOrReturnError(nullptr != storage, CHIP_ERROR_INVALID_ARGUMENT);

        StorageKeyName key = StorageKeyName::Uninitialized();
        ReturnErrorOnFailure(persistent.UpdateKey(key));

        return storage->SyncDeleteKeyValue(key.KeyName());
    }

    uint8_t mBuffer[kMaxSerializedSize] = { 0 };
};

/// @brief Combines PersistentStore and DataAccessor
template <size_t kMaxSerializedSize>
struct PersistentData : PersistentStore<kMaxSerializedSize>, DataAccessor
{
    using SizedStore = PersistentStore<kMaxSerializedSize>;

    PersistentData(PersistentStorageDelegate * storage = nullptr) : mStorage(storage) {}
    virtual ~PersistentData() = default;

    virtual CHIP_ERROR Save() { return this->Save(this->mStorage); }

    virtual CHIP_ERROR Save(PersistentStorageDelegate * storage) { return SizedStore::Save(*this, storage); }

    virtual CHIP_ERROR Load() { return this->Load(this->mStorage); }

    virtual CHIP_ERROR Load(PersistentStorageDelegate * storage) { return SizedStore::Load(*this, storage); }

    virtual CHIP_ERROR Delete(PersistentStorageDelegate * storage) { return SizedStore::Delete(*this, storage); }

    PersistentStorageDelegate * mStorage = nullptr;
};

} // namespace chip
