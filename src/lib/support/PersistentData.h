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

/// @brief Interface to Persistent Storage Delegate allowing storage of data of variable size such as TLV.
/// @tparam kMaxSerializedSize size of the mBuffer necessary to retrieve an entry from the storage. Varies with the type of data
/// stored. Will be allocated on the stack so the implementation needs to be aware of this when choosing this value.
template <size_t kMaxSerializedSize>
struct PersistentData
{
    PersistentData(PersistentStorageDelegate * storage = nullptr) : mStorage(storage) {}
    virtual ~PersistentData() = default;

    virtual CHIP_ERROR UpdateKey(StorageKeyName & key)          = 0;
    virtual CHIP_ERROR Serialize(TLV::TLVWriter & writer) const = 0;
    virtual CHIP_ERROR Deserialize(TLV::TLVReader & reader)     = 0;
    virtual void Clear()                                        = 0;

    virtual CHIP_ERROR Save() { return this->Save(this->mStorage); }

    virtual CHIP_ERROR Save(PersistentStorageDelegate * storage)
    {
        VerifyOrReturnError(nullptr != storage, CHIP_ERROR_INVALID_ARGUMENT);

        StorageKeyName key = StorageKeyName::Uninitialized();
        ReturnErrorOnFailure(UpdateKey(key));

        // Serialize the data
        TLV::TLVWriter writer;
        writer.Init(mBuffer, sizeof(mBuffer));

        ReturnErrorOnFailure(Serialize(writer));

        // Save serialized data
        return storage->SyncSetKeyValue(key.KeyName(), mBuffer, static_cast<uint16_t>(writer.GetLengthWritten()));
    }

    virtual CHIP_ERROR Load() { return this->Load(this->mStorage); }

    virtual CHIP_ERROR Load(PersistentStorageDelegate * storage)
    {
        VerifyOrReturnError(nullptr != storage, CHIP_ERROR_INVALID_ARGUMENT);

        StorageKeyName key = StorageKeyName::Uninitialized();

        // Update storage key
        ReturnErrorOnFailure(UpdateKey(key));

        // Set data to defaults
        Clear();

        // Load the serialized data
        uint16_t size  = static_cast<uint16_t>(sizeof(mBuffer));
        CHIP_ERROR err = storage->SyncGetKeyValue(key.KeyName(), mBuffer, size);
        VerifyOrReturnError(CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND != err, CHIP_ERROR_NOT_FOUND);
        ReturnErrorOnFailure(err);

        // Decode serialized data
        TLV::TLVReader reader;
        reader.Init(mBuffer, size);
        return Deserialize(reader);
    }

    virtual CHIP_ERROR Delete(PersistentStorageDelegate * storage)
    {
        VerifyOrReturnError(nullptr != storage, CHIP_ERROR_INVALID_ARGUMENT);

        StorageKeyName key = StorageKeyName::Uninitialized();
        ReturnErrorOnFailure(UpdateKey(key));

        return storage->SyncDeleteKeyValue(key.KeyName());
    }

    PersistentStorageDelegate * mStorage = nullptr;
    uint8_t mBuffer[kMaxSerializedSize]  = { 0 };
};

} // namespace chip
