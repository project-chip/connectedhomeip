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
#include <lib/support/Span.h>

namespace chip {

/// @brief Data that can be persisted via PersistentStorageDelegate in TLV format.
struct DataAccessor
{
    virtual ~DataAccessor()                                     = default;
    virtual CHIP_ERROR UpdateKey(StorageKeyName & key) const    = 0;
    virtual CHIP_ERROR Serialize(TLV::TLVWriter & writer) const = 0;
    virtual CHIP_ERROR Deserialize(TLV::TLVReader & reader)     = 0;
    virtual void Clear()                                        = 0;

    /// Non-virtual helper methods ///

    CHIP_ERROR Save(PersistentStorageDelegate * storage, const MutableByteSpan & buffer) const
    {
        VerifyOrReturnError(nullptr != storage, CHIP_ERROR_INVALID_ARGUMENT);

        StorageKeyName key = StorageKeyName::Uninitialized();
        ReturnErrorOnFailure(this->UpdateKey(key));

        // Serialize the data
        TLV::TLVWriter writer;
        writer.Init(buffer);
        ReturnErrorOnFailure(this->Serialize(writer));

        // Save serialized data
        return storage->SyncSetKeyValue(key.KeyName(), buffer.data(), static_cast<uint16_t>(writer.GetLengthWritten()));
    }

    CHIP_ERROR Load(PersistentStorageDelegate * storage, const MutableByteSpan & buffer)
    {
        VerifyOrReturnError(nullptr != storage, CHIP_ERROR_INVALID_ARGUMENT);

        StorageKeyName key = StorageKeyName::Uninitialized();
        ReturnErrorOnFailure(this->UpdateKey(key));

        // Set data to defaults
        this->Clear();

        // Load the serialized data
        uint16_t size  = (buffer.size() > UINT16_MAX) ? UINT16_MAX : static_cast<uint16_t>(buffer.size());
        CHIP_ERROR err = storage->SyncGetKeyValue(key.KeyName(), buffer.data(), size);
        VerifyOrReturnError(CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND != err, CHIP_ERROR_NOT_FOUND);
        ReturnErrorOnFailure(err);

        // Decode serialized data
        TLV::TLVReader reader;
        reader.Init(buffer.data(), size);
        return this->Deserialize(reader);
    }

    CHIP_ERROR Delete(PersistentStorageDelegate * storage) const
    {
        VerifyOrReturnError(nullptr != storage, CHIP_ERROR_INVALID_ARGUMENT);

        StorageKeyName key = StorageKeyName::Uninitialized();
        ReturnErrorOnFailure(this->UpdateKey(key));

        return storage->SyncDeleteKeyValue(key.KeyName());
    }
};

/// @brief A buffer to be used when loading or serializing persistent data
/// @tparam kMaxSerializedSize size of the buffer necessary to retrieve an entry from the storage.
///
/// Note: Generic APIs that utilize a persistence buffer should prefer taking
/// a MutableByteSpan, to avoid unnecessarily templating on the buffer size.
template <size_t kMaxSerializedSize>
struct PersistenceBuffer
{
    uint8_t mBuffer[kMaxSerializedSize];

    // Returns a MutableByteSpan representing this buffer.
    MutableByteSpan BufferSpan() { return MutableByteSpan(mBuffer); }
};

/// @brief Combines a PersistenceBuffer with DataAccessor helpers.
/// Only exists for compatibility with existing code;
/// new code should use PersistenceBuffer and DataAccessor separately.
template <size_t kMaxSerializedSize>
struct [[deprecated("Use PersistenceBuffer and DataAccessor separately")]] PersistentStore
    : public PersistenceBuffer<kMaxSerializedSize>
{
    virtual ~PersistentStore() = default;

    CHIP_ERROR Save(const DataAccessor & persistent, PersistentStorageDelegate * storage)
    {
        return persistent.Save(storage, this->BufferSpan());
    }

    CHIP_ERROR Load(DataAccessor & persistent, PersistentStorageDelegate * storage)
    {
        return persistent.Load(storage, this->BufferSpan());
    }

    CHIP_ERROR Delete(DataAccessor & persistent, PersistentStorageDelegate * storage) { return persistent.Delete(storage); }
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
