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
#include <lib/core/CHIPTLV.h>
#include <lib/support/DefaultStorageKeyAllocator.h>

namespace chip {

constexpr size_t kPersistentBufferMax = 128;

template <size_t kMaxSerializedSize>
struct PersistentData
{
    virtual ~PersistentData() = default;

    virtual CHIP_ERROR UpdateKey(DefaultStorageKeyAllocator & key) = 0;
    virtual CHIP_ERROR Serialize(TLV::TLVWriter & writer) const    = 0;
    virtual CHIP_ERROR Deserialize(TLV::TLVReader & reader)        = 0;
    virtual void Clear()                                           = 0;

    virtual CHIP_ERROR Save(PersistentStorageDelegate * storage)
    {
        VerifyOrReturnError(nullptr != storage, CHIP_ERROR_INVALID_ARGUMENT);

        uint8_t buffer[kMaxSerializedSize] = { 0 };
        DefaultStorageKeyAllocator key;
        // Update storage key
        ReturnErrorOnFailure(UpdateKey(key));

        // Serialize the data
        TLV::TLVWriter writer;
        writer.Init(buffer, sizeof(buffer));
        ReturnErrorOnFailure(Serialize(writer));

        // Save serialized data
        return storage->SyncSetKeyValue(key.KeyName(), buffer, static_cast<uint16_t>(writer.GetLengthWritten()));
    }

    CHIP_ERROR Load(PersistentStorageDelegate * storage)
    {
        VerifyOrReturnError(nullptr != storage, CHIP_ERROR_INVALID_ARGUMENT);

        uint8_t buffer[kMaxSerializedSize] = { 0 };
        DefaultStorageKeyAllocator key;

        // Set data to defaults
        Clear();

        // Update storage key
        ReturnErrorOnFailure(UpdateKey(key));

        // Load the serialized data
        uint16_t size  = static_cast<uint16_t>(sizeof(buffer));
        CHIP_ERROR err = storage->SyncGetKeyValue(key.KeyName(), buffer, size);
        VerifyOrReturnError(CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND != err, CHIP_ERROR_NOT_FOUND);
        ReturnErrorOnFailure(err);

        // Decode serialized data
        TLV::TLVReader reader;
        reader.Init(buffer, size);
        return Deserialize(reader);
    }

    virtual CHIP_ERROR Delete(PersistentStorageDelegate * storage)
    {
        VerifyOrReturnError(nullptr != storage, CHIP_ERROR_INVALID_ARGUMENT);

        DefaultStorageKeyAllocator key;
        // Update storage key
        ReturnErrorOnFailure(UpdateKey(key));
        // Delete stored data
        return storage->SyncDeleteKeyValue(key.KeyName());
    }
};

struct FabricList : public PersistentData<kPersistentBufferMax>
{
    static constexpr TLV::Tag TagFirstFabric() { return TLV::ContextTag(1); }
    static constexpr TLV::Tag TagFabricCount() { return TLV::ContextTag(2); }

    chip::FabricIndex first_fabric = kUndefinedFabricIndex;
    uint8_t fabric_count           = 0;

    FabricList() = default;
    FabricList(chip::FabricIndex first) : first_fabric(first), fabric_count(1) {}

    CHIP_ERROR UpdateKey(DefaultStorageKeyAllocator & key) override
    {
        key.FabricList();
        return CHIP_NO_ERROR;
    }

    void Clear() override
    {
        first_fabric = kUndefinedFabricIndex;
        fabric_count = 0;
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));

        ReturnErrorOnFailure(writer.Put(TagFirstFabric(), static_cast<uint16_t>(first_fabric)));
        ReturnErrorOnFailure(writer.Put(TagFabricCount(), static_cast<uint16_t>(fabric_count)));

        return writer.EndContainer(container);
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
        VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_INTERNAL);

        TLV::TLVType container;
        ReturnErrorOnFailure(reader.EnterContainer(container));

        // first_fabric
        ReturnErrorOnFailure(reader.Next(TagFirstFabric()));
        ReturnErrorOnFailure(reader.Get(first_fabric));
        // fabric_count
        ReturnErrorOnFailure(reader.Next(TagFabricCount()));
        ReturnErrorOnFailure(reader.Get(fabric_count));

        return reader.ExitContainer(container);
    }
};

/**
 * Template used to iterate the stored data
 */
template <typename T>
class Iterator
{
public:
    virtual ~Iterator() = default;
    /**
     *  @retval The number of entries in total that will be iterated.
     */
    virtual size_t Count() = 0;
    /**
     *   @param[out] item  Value associated with the next element in the iteration.
     *  @retval true if the next entry is successfully retrieved.
     *  @retval false if no more entries can be found.
     */
    virtual bool Next(T & item) = 0;
    /**
     * Release the memory allocated by this iterator.
     * Must be called before the pointer goes out of scope.
     */
    virtual void Release() = 0;

protected:
    Iterator() = default;
};

} // namespace chip