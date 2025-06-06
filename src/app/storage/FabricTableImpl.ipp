/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/storage/FabricTableImpl.h>
#include <app/util/endpoint-config-api.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/TypeTraits.h>
#include <stdlib.h>

namespace chip {
namespace app {
namespace Storage {

using EntryIndex = Data::EntryIndex;

/// @brief Tags Used to serialize entries so they can be stored in flash memory.
/// kEndpointEntryCount: Number of entries in an endpoint
/// kEntryCount: Number of entries in a Fabric
/// kStorageIdArray: Array of StorageId struct
enum class TagEntry : uint8_t
{
    kEndpointEntryCount = 1,
    kEntryCount,
    kStorageIdArray,
    kFabricTableFirstSpecializationReservedTag,
    kFabricTableLastSpecializationReservedTag = 127,
    // Add new entries here; kFabricTableFirstSpecializationReservedTag through
    // kFabricTableLastSpecializationReservedTag are reserved for specializations
};

// Currently takes 5 Bytes to serialize Container and value in a TLV: 1 byte start struct, 2 bytes control + tag for the value, 1
// byte value, 1 byte end struct. 8 Bytes leaves space for potential increase in count_value size.
static constexpr size_t kPersistentBufferEntryCountBytes = 8;

template <class StorageId, class StorageData>
struct EndpointEntryCount : public PersistentData<kPersistentBufferEntryCountBytes>
{
    using Serializer = DefaultSerializer<StorageId, StorageData>;

    EndpointId endpoint_id = kInvalidEndpointId;
    uint8_t count_value    = 0;

    EndpointEntryCount(EndpointId endpoint, uint8_t count = 0) : endpoint_id(endpoint), count_value(count) {}
    ~EndpointEntryCount() {}

    void Clear() override { count_value = 0; }

    CHIP_ERROR UpdateKey(StorageKeyName & key) const override
    {
        VerifyOrReturnError(kInvalidEndpointId != endpoint_id, CHIP_ERROR_INVALID_ARGUMENT);
        key = Serializer::EndpointEntryCountKey(endpoint_id);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagEntry::kEndpointEntryCount), count_value));
        return writer.EndContainer(container);
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

        TLV::TLVType container;
        ReturnErrorOnFailure(reader.EnterContainer(container));
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagEntry::kEndpointEntryCount)));
        ReturnErrorOnFailure(reader.Get(count_value));
        return reader.ExitContainer(container);
    }

    CHIP_ERROR Load(PersistentStorageDelegate * storage) override
    {
        CHIP_ERROR err = PersistentData::Load(storage);
        VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);
        if (CHIP_ERROR_NOT_FOUND == err)
        {
            count_value = 0;
        }

        return CHIP_NO_ERROR;
    }
};

// Prevent mutations from happening in TableEntryData::Serialize
// If we just used a raw reference for TableEntryData::mEntry, C++ allows us
// to mutate mEntry.mStorageId & mEntry.mStorageData in TableEntryData::Serialize
// without having to do a const_cast; as an example, if we were to accidentally introduce
// the following code in TableEntryData::Serialize (a const method):
//
// this->mEntry->mStorageData = StorageData();
//
// If TableEntryData::mEntry is a reference, it allows this with no compilation error;
// But with ConstCorrectRef, we get a compile-time error that TableEntryData::mEntry->mStorageData
// cannot be modified because it is a const value
template <typename T>
class ConstCorrectRef
{
    T & mRef;

public:
    inline ConstCorrectRef(T & ref) : mRef(ref) {}

    inline const T * operator->() const { return &mRef; }
    inline T * operator->() { return &mRef; }

    inline const T & operator*() const { return mRef; }
    inline T & operator*() { return mRef; }
};

template <class StorageId, class StorageData>
struct TableEntryData : DataAccessor
{
    using Serializer = DefaultSerializer<StorageId, StorageData>;

    EndpointId endpoint_id   = kInvalidEndpointId;
    FabricIndex fabric_index = kUndefinedFabricIndex;
    EntryIndex index         = 0;
    bool first               = true;
    ConstCorrectRef<StorageId> storage_id;
    ConstCorrectRef<StorageData> storage_data;

    TableEntryData(EndpointId endpoint, FabricIndex fabric, StorageId & id, StorageData & data, EntryIndex idx = 0) :
        endpoint_id(endpoint), fabric_index(fabric), index(idx), storage_id(id), storage_data(data)
    {}

    CHIP_ERROR UpdateKey(StorageKeyName & key) const override
    {
        VerifyOrReturnError(kUndefinedFabricIndex != fabric_index, CHIP_ERROR_INVALID_FABRIC_INDEX);
        VerifyOrReturnError(kInvalidEndpointId != endpoint_id, CHIP_ERROR_INVALID_ARGUMENT);
        key = Serializer::FabricEntryKey(fabric_index, endpoint_id, index);
        return CHIP_NO_ERROR;
    }

    void Clear() override { Serializer::Clear(*storage_data); }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));

        ReturnErrorOnFailure(Serializer::SerializeId(writer, *storage_id));

        ReturnErrorOnFailure(Serializer::SerializeData(writer, *storage_data));

        return writer.EndContainer(container);
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

        TLV::TLVType container;
        ReturnErrorOnFailure(reader.EnterContainer(container));

        ReturnErrorOnFailure(Serializer::DeserializeId(reader, *storage_id));

        ReturnErrorOnFailure(Serializer::DeserializeData(reader, *storage_data));

        return reader.ExitContainer(container);
    }
};

/**
 * @brief Class that holds a map to all entries in a fabric for a specific endpoint
 *
 * FabricEntryData is an access to a linked list of entries
 */
template <class StorageId, class StorageData, size_t kEntryMaxBytes, size_t kFabricMaxBytes, uint16_t kMaxPerFabric>
struct FabricEntryData : public PersistentData<kFabricMaxBytes>
{
    using Serializer              = DefaultSerializer<StorageId, StorageData>;
    using TypedTableEntryData     = TableEntryData<StorageId, StorageData>;
    using Store                   = PersistentStore<kEntryMaxBytes>;
    using TypedEndpointEntryCount = EndpointEntryCount<StorageId, StorageData>;

    EndpointId endpoint_id;
    FabricIndex fabric_index;
    uint8_t entry_count = 0;
    uint16_t max_per_fabric;
    uint16_t max_per_endpoint;
    StorageId entry_map[kMaxPerFabric];

    FabricEntryData(EndpointId endpoint = kInvalidEndpointId, FabricIndex fabric = kUndefinedFabricIndex,
                    uint16_t maxPerFabric = kMaxPerFabric, uint16_t maxPerEndpoint = Serializer::kMaxPerEndpoint()) :
        endpoint_id(endpoint),
        fabric_index(fabric), max_per_fabric(maxPerFabric), max_per_endpoint(maxPerEndpoint)
    {}

    CHIP_ERROR UpdateKey(StorageKeyName & key) const override
    {
        VerifyOrReturnError(kUndefinedFabricIndex != fabric_index, CHIP_ERROR_INVALID_FABRIC_INDEX);
        VerifyOrReturnError(kInvalidEndpointId != endpoint_id, CHIP_ERROR_INVALID_ARGUMENT);
        key = Serializer::FabricEntryDataKey(fabric_index, endpoint_id);
        return CHIP_NO_ERROR;
    }

    void Clear() override
    {
        entry_count = 0;
        for (uint16_t i = 0; i < max_per_fabric; i++)
        {
            entry_map[i].Clear();
        }
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        TLV::TLVType fabricEntryContainer;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, fabricEntryContainer));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagEntry::kEntryCount), entry_count));
        TLV::TLVType entryMapContainer;
        ReturnErrorOnFailure(
            writer.StartContainer(TLV::ContextTag(TagEntry::kStorageIdArray), TLV::kTLVType_Array, entryMapContainer));

        // Storing the entry map
        for (uint16_t i = 0; i < max_per_fabric; i++)
        {
            TLV::TLVType entryIdContainer;
            ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, entryIdContainer));
            ReturnErrorOnFailure(Serializer::SerializeId(writer, entry_map[i]));
            ReturnErrorOnFailure(writer.EndContainer(entryIdContainer));
        }
        ReturnErrorOnFailure(writer.EndContainer(entryMapContainer));
        return writer.EndContainer(fabricEntryContainer);
    }

    /// @brief This Deserialize method is implemented only to allow compilation. It is not used throughout the code.
    /// @param reader TLV reader
    /// @return CHIP_NO_ERROR
    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override { return CHIP_ERROR_INCORRECT_STATE; }

    /// @brief This Deserialize method checks that the recovered entries from the deserialization fit in the current max and if
    /// there are too many entries in nvm, it deletes them. The method sets the deleted_entries output parameter to true if entries
    /// were deleted so that the load function can know it needs to save the Fabric entry data to update the entry_count and the
    /// entry map in stored memory.
    /// @param reade [in] TLV reader, must be big enough to hold the entry size
    /// @param storage [in] Persistent Storage Delegate, required to delete entries if the number of entries in storage is greater
    /// than the maximum allowed
    /// @param deleted_entries_count [out] uint8_t letting the caller (in this case the load method) know how many entries were
    /// deleted so it can adjust the fabric and global entry count accordingly. Even if Deserialize fails, this value will return
    /// the number of entries deleted before the failure happened.
    /// @return CHIP_NO_ERROR on success, specific CHIP_ERROR otherwise
    CHIP_ERROR Deserialize(TLV::TLVReader & reader, PersistentStorageDelegate & storage, uint8_t & deleted_entries_count)
    {
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
        TLV::TLVType fabricEntryContainer;
        ReturnErrorOnFailure(reader.EnterContainer(fabricEntryContainer));
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagEntry::kEntryCount)));
        ReturnErrorOnFailure(reader.Get(entry_count));
        entry_count = std::min(entry_count, static_cast<uint8_t>(max_per_fabric));
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::ContextTag(TagEntry::kStorageIdArray)));
        TLV::TLVType entryMapContainer;
        ReturnErrorOnFailure(reader.EnterContainer(entryMapContainer));

        uint16_t i = 0;
        CHIP_ERROR err;
        deleted_entries_count = 0;

        Store persistentStore;
        while ((err = reader.Next(TLV::AnonymousTag())) == CHIP_NO_ERROR)
        {
            TLV::TLVType entryIdContainer;
            if (i < max_per_fabric)
            {
                ReturnErrorOnFailure(reader.EnterContainer(entryIdContainer));
                ReturnErrorOnFailure(Serializer::DeserializeId(reader, entry_map[i]));
                ReturnErrorOnFailure(reader.ExitContainer(entryIdContainer));
            }
            else
            {
                StorageId unused;
                ReturnErrorOnFailure(reader.EnterContainer(entryIdContainer));
                ReturnErrorOnFailure(Serializer::DeserializeId(reader, unused));
                ReturnErrorOnFailure(reader.ExitContainer(entryIdContainer));
                ReturnErrorOnFailure(DeleteValue(storage, i));
                deleted_entries_count++;
            }

            i++;
        }

        VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
        ReturnErrorOnFailure(reader.ExitContainer(entryMapContainer));
        return reader.ExitContainer(fabricEntryContainer);
    }

    /// @brief  Finds the id of the entry with the specified index
    /// @return CHIP_NO_ERROR if managed to find the target entry, CHIP_ERROR_NOT_FOUND if not found
    CHIP_ERROR FindByIndex(PersistentStorageDelegate & storage, EntryIndex index, StorageId & entry_id)
    {
        VerifyOrReturnError(entry_map[index].IsValid(), CHIP_ERROR_NOT_FOUND);
        VerifyOrReturnError(kUndefinedFabricIndex != fabric_index, CHIP_ERROR_INVALID_FABRIC_INDEX);
        VerifyOrReturnError(kInvalidEndpointId != endpoint_id, CHIP_ERROR_INVALID_ARGUMENT);
        if (!storage.SyncDoesKeyExist(Serializer::FabricEntryKey(fabric_index, endpoint_id, index).KeyName()))
        {
            return CHIP_ERROR_NOT_FOUND;
        }
        entry_id = entry_map[index];
        return CHIP_NO_ERROR;
    }

    /// @brief  Finds the index where the current entry should be inserted by going through the endpoint's table and checking
    /// whether the entry is already there. If the target is not in the table, sets idx to the first empty space
    /// @param target_entry StorageId of entry to find
    /// @param idx Index where target or space is found
    /// @return CHIP_NO_ERROR if managed to find the target entry, CHIP_ERROR_NOT_FOUND if not found and space left
    ///         CHIP_ERROR_NO_MEMORY if target was not found and table is full
    CHIP_ERROR Find(const StorageId & target_entry, EntryIndex & idx)
    {
        EntryIndex firstFreeIdx = Data::kUndefinedEntryIndex; // storage index if entry not found
        uint16_t index          = 0;

        while (index < max_per_fabric)
        {
            if (entry_map[index] == target_entry)
            {
                idx = index;
                return CHIP_NO_ERROR; // return entry at current index if entry found
            }
            if (!entry_map[index].IsValid() && firstFreeIdx == Data::kUndefinedEntryIndex)
            {
                firstFreeIdx = index;
            }
            index++;
        }

        if (firstFreeIdx < max_per_fabric)
        {
            idx = firstFreeIdx;
            return CHIP_ERROR_NOT_FOUND;
        }

        return CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR SaveEntry(PersistentStorageDelegate & storage, const StorageId & id, const StorageData & data)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        // Look for empty storage space

        EntryIndex index;
        err = this->Find(id, index);

        Store persistentStore;
        // C++ doesn't have const constructors; variable is declared const
        const TypedTableEntryData entry(endpoint_id, fabric_index, const_cast<StorageId &>(id), const_cast<StorageData &>(data),
                                        index);

        if (CHIP_NO_ERROR == err)
        {
            return persistentStore.Save(entry, &storage);
        }

        if (CHIP_ERROR_NOT_FOUND == err) // If not found, entry.index should be the first free index
        {
            // Update the global entry count
            TypedEndpointEntryCount endpoint_count(endpoint_id);
            ReturnErrorOnFailure(endpoint_count.Load(&storage));
            VerifyOrReturnError(endpoint_count.count_value < max_per_endpoint, CHIP_ERROR_NO_MEMORY);
            endpoint_count.count_value++;
            ReturnErrorOnFailure(endpoint_count.Save(&storage));

            entry_count++;
            entry_map[entry.index] = id;

            err = this->Save(&storage);
            if (CHIP_NO_ERROR != err)
            {
                endpoint_count.count_value--;
                ReturnErrorOnFailure(endpoint_count.Save(&storage));
                return err;
            }

            err = persistentStore.Save(entry, &storage);

            // on failure to save the entry, undoes the changes to Fabric Entry Data
            if (CHIP_NO_ERROR != err)
            {
                endpoint_count.count_value--;
                ReturnErrorOnFailure(endpoint_count.Save(&storage));

                entry_count--;
                entry_map[entry.index].Clear();
                ReturnErrorOnFailure(this->Save(&storage));
                return err;
            }
        }

        return err;
    }

    /// @brief Removes an entry from the non-volatile memory and clears its index in the entry map. Decreases the number of entries
    /// in the global entry count and in the entry fabric data if successful. As the entry map size is not compressed upon removal,
    /// this only clears the entry corresponding to the entry from the entry map.
    /// @param storage Storage delegate to access the entry
    /// @param entry_id Entry to remove
    /// @return CHIP_NO_ERROR if successful, specific CHIP_ERROR otherwise
    CHIP_ERROR RemoveEntry(PersistentStorageDelegate & storage, const StorageId & entry_id)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        EntryIndex entryIndex;

        // Empty Entry Fabric Data returns CHIP_NO_ERROR on remove
        if (entry_count > 0)
        {
            // If Find doesn't return CHIP_NO_ERROR, the entry wasn't found, which doesn't return an error
            VerifyOrReturnValue(this->Find(entry_id, entryIndex) == CHIP_NO_ERROR, CHIP_NO_ERROR);

            // Update the global entry count
            TypedEndpointEntryCount endpoint_entry_count(endpoint_id);
            ReturnErrorOnFailure(endpoint_entry_count.Load(&storage));
            endpoint_entry_count.count_value--;
            ReturnErrorOnFailure(endpoint_entry_count.Save(&storage));

            entry_count--;
            entry_map[entryIndex].Clear();
            err = this->Save(&storage);

            // On failure to update the entry map, undo the global count modification
            if (CHIP_NO_ERROR != err)
            {
                endpoint_entry_count.count_value++;
                ReturnErrorOnFailure(endpoint_entry_count.Save(&storage));
                return err;
            }

            err = DeleteValue(storage, entryIndex);

            // On failure to delete entry, undo the change to the Fabric Entry Data and the global entry count
            if (CHIP_NO_ERROR != err)
            {
                endpoint_entry_count.count_value++;
                ReturnErrorOnFailure(endpoint_entry_count.Save(&storage));

                entry_count++;
                entry_map[entryIndex] = entry_id;
                ReturnErrorOnFailure(this->Save(&storage));
                return err;
            }
        }
        return err;
    }

    CHIP_ERROR Load(PersistentStorageDelegate * storage) override
    {
        VerifyOrReturnError(nullptr != storage, CHIP_ERROR_INVALID_ARGUMENT);
        uint8_t deleted_entries_count = 0;

        uint8_t buffer[kFabricMaxBytes] = { 0 };
        StorageKeyName key              = StorageKeyName::Uninitialized();

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

        err = Deserialize(reader, *storage, deleted_entries_count);

        // If Deserialize sets the "deleted_entries" variable, the table in flash memory held too many entries (can happen
        // if max_per_fabric was reduced during an OTA) and was adjusted during deserializing . The fabric data must then
        // be updated
        if (deleted_entries_count)
        {
            TypedEndpointEntryCount global_count(endpoint_id);
            ReturnErrorOnFailure(global_count.Load(storage));
            global_count.count_value = static_cast<uint8_t>(global_count.count_value - deleted_entries_count);
            ReturnErrorOnFailure(global_count.Save(storage));
            ReturnErrorOnFailure(this->Save(storage));
        }

        return err;
    }

private:
    CHIP_ERROR DeleteValue(PersistentStorageDelegate & storage, EntryIndex index)
    {
        StorageKeyName key = Serializer::FabricEntryKey(fabric_index, endpoint_id, index);
        return storage.SyncDeleteKeyValue(key.KeyName());
    }
};

template <class StorageId, class StorageData, size_t kIteratorsMax>
CHIP_ERROR FabricTableImpl<StorageId, StorageData, kIteratorsMax>::Init(PersistentStorageDelegate & storage)
{
    // Verify the initialized parameter respects the maximum allowed values for entry capacity
    VerifyOrReturnError(mMaxPerFabric <= Serializer::kMaxPerFabric() && mMaxPerEndpoint <= Serializer::kMaxPerEndpoint(),
                        CHIP_ERROR_INVALID_INTEGER_VALUE);
    this->mStorage = &storage;
    return CHIP_NO_ERROR;
}

template <class StorageId, class StorageData, size_t kIteratorsMax>
void FabricTableImpl<StorageId, StorageData, kIteratorsMax>::Finish()
{}

template <class StorageId, class StorageData, size_t kIteratorsMax>
CHIP_ERROR FabricTableImpl<StorageId, StorageData, kIteratorsMax>::GetFabricEntryCount(FabricIndex fabric_index,
                                                                                       uint8_t & entry_count)
{
    using TypedFabricEntryData = FabricEntryData<StorageId, StorageData, Serializer::kEntryMaxBytes(),
                                                 Serializer::kFabricMaxBytes(), Serializer::kMaxPerFabric()>;

    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    TypedFabricEntryData fabric(mEndpointId, fabric_index);
    CHIP_ERROR err = fabric.Load(mStorage);
    VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);

    entry_count = (CHIP_ERROR_NOT_FOUND == err) ? 0 : fabric.entry_count;

    return CHIP_NO_ERROR;
}

template <class StorageId, class StorageData, size_t kIteratorsMax>
CHIP_ERROR FabricTableImpl<StorageId, StorageData, kIteratorsMax>::GetEndpointEntryCount(uint8_t & entry_count)
{
    using TypedEndpointEntryCount = EndpointEntryCount<StorageId, StorageData>;

    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    TypedEndpointEntryCount endpoint_entry_count(mEndpointId);

    ReturnErrorOnFailure(endpoint_entry_count.Load(mStorage));
    entry_count = endpoint_entry_count.count_value;

    return CHIP_NO_ERROR;
}

template <class StorageId, class StorageData, size_t kIteratorsMax>
CHIP_ERROR FabricTableImpl<StorageId, StorageData, kIteratorsMax>::SetEndpointEntryCount(const uint8_t & entry_count)
{
    using TypedEndpointEntryCount = EndpointEntryCount<StorageId, StorageData>;
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    TypedEndpointEntryCount endpoint_entry_count(mEndpointId, entry_count);
    return endpoint_entry_count.Save(mStorage);
}

template <class StorageId, class StorageData, size_t kIteratorsMax>
CHIP_ERROR FabricTableImpl<StorageId, StorageData, kIteratorsMax>::GetRemainingCapacity(FabricIndex fabric_index,
                                                                                        uint8_t & capacity)
{
    using TypedFabricEntryData = FabricEntryData<StorageId, StorageData, Serializer::kEntryMaxBytes(),
                                                 Serializer::kFabricMaxBytes(), Serializer::kMaxPerFabric()>;

    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    uint8_t endpoint_entry_count = 0;
    ReturnErrorOnFailure(GetEndpointEntryCount(endpoint_entry_count));

    // If the global entry count is higher than the maximal Global entry capacity, this returns a capacity of 0 until enough entries
    // have been deleted to bring the global number of entries under the global maximum.
    if (endpoint_entry_count > mMaxPerEndpoint)
    {
        capacity = 0;
        return CHIP_NO_ERROR;
    }
    uint8_t remaining_capacity_global = static_cast<uint8_t>(mMaxPerEndpoint - endpoint_entry_count);
    uint8_t remaining_capacity_fabric = static_cast<uint8_t>(mMaxPerFabric);

    TypedFabricEntryData fabric(mEndpointId, fabric_index);

    // Load fabric data (defaults to zero)TypedFabricEntryData
    CHIP_ERROR err = fabric.Load(mStorage);
    VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);

    if (err == CHIP_NO_ERROR)
    {
        remaining_capacity_fabric = static_cast<uint8_t>(mMaxPerFabric - fabric.entry_count);
    }

    capacity = std::min(remaining_capacity_fabric, remaining_capacity_global);

    return CHIP_NO_ERROR;
}

template <class StorageId, class StorageData, size_t kIteratorsMax>
CHIP_ERROR FabricTableImpl<StorageId, StorageData, kIteratorsMax>::SetTableEntry(FabricIndex fabric_index, const StorageId & id,
                                                                                 const StorageData & data)
{
    using TypedFabricEntryData = FabricEntryData<StorageId, StorageData, Serializer::kEntryMaxBytes(),
                                                 Serializer::kFabricMaxBytes(), Serializer::kMaxPerFabric()>;

    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    TypedFabricEntryData fabric(mEndpointId, fabric_index, mMaxPerFabric, mMaxPerEndpoint);

    // Load fabric data (defaults to zero)
    CHIP_ERROR err = fabric.Load(mStorage);
    VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);

    err = fabric.SaveEntry(*mStorage, id, data);
    return err;
}

template <class StorageId, class StorageData, size_t kIteratorsMax>
template <size_t kEntryMaxBytes>
CHIP_ERROR FabricTableImpl<StorageId, StorageData, kIteratorsMax>::GetTableEntry(FabricIndex fabric_index, StorageId & entry_id,
                                                                                 StorageData & data,
                                                                                 PersistentStore<kEntryMaxBytes> & buffer)
{
    using TypedFabricEntryData = FabricEntryData<StorageId, StorageData, Serializer::kEntryMaxBytes(),
                                                 Serializer::kFabricMaxBytes(), Serializer::kMaxPerFabric()>;
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    TypedFabricEntryData fabric(mEndpointId, fabric_index, mMaxPerFabric, mMaxPerEndpoint);
    TableEntryData<StorageId, StorageData> table_entry(mEndpointId, fabric_index, entry_id, data);

    ReturnErrorOnFailure(fabric.Load(mStorage));
    VerifyOrReturnError(fabric.Find(entry_id, table_entry.index) == CHIP_NO_ERROR, CHIP_ERROR_NOT_FOUND);

    CHIP_ERROR err = buffer.Load(table_entry, mStorage);

    // If entry.Load returns "buffer too small", the entry in memory is too big to be retrieved (this could happen if the
    // kEntryMaxBytes was reduced by OTA) and therefore must be deleted as is is no longer considered accessible.
    if (err == CHIP_ERROR_BUFFER_TOO_SMALL)
    {
        ReturnErrorOnFailure(this->RemoveTableEntry(fabric_index, entry_id));
    }
    ReturnErrorOnFailure(err);

    return CHIP_NO_ERROR;
}

template <class StorageId, class StorageData, size_t kIteratorsMax>
CHIP_ERROR FabricTableImpl<StorageId, StorageData, kIteratorsMax>::FindTableEntry(FabricIndex fabric_index,
                                                                                  const StorageId & entry_id, EntryIndex & idx)
{
    using TypedFabricEntryData = FabricEntryData<StorageId, StorageData, Serializer::kEntryMaxBytes(),
                                                 Serializer::kFabricMaxBytes(), Serializer::kMaxPerFabric()>;
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    TypedFabricEntryData fabric(mEndpointId, fabric_index, mMaxPerFabric, mMaxPerEndpoint);

    ReturnErrorOnFailure(fabric.Load(mStorage));
    VerifyOrReturnError(fabric.Find(entry_id, idx) == CHIP_NO_ERROR, CHIP_ERROR_NOT_FOUND);

    return CHIP_NO_ERROR;
}

template <class StorageId, class StorageData, size_t kIteratorsMax>
CHIP_ERROR FabricTableImpl<StorageId, StorageData, kIteratorsMax>::RemoveTableEntry(FabricIndex fabric_index,
                                                                                    const StorageId & entry_id)
{
    using TypedFabricEntryData = FabricEntryData<StorageId, StorageData, Serializer::kEntryMaxBytes(),
                                                 Serializer::kFabricMaxBytes(), Serializer::kMaxPerFabric()>;

    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);
    TypedFabricEntryData fabric(mEndpointId, fabric_index, mMaxPerFabric, mMaxPerEndpoint);

    ReturnErrorOnFailure(fabric.Load(mStorage));

    return fabric.RemoveEntry(*mStorage, entry_id);
}

/// @brief This function is meant to provide a way to empty the entry table without knowing any specific entry Id. Outside of this
/// specific use case, RemoveTableEntry should be used.
/// @param fabric_index Fabric in which the entry belongs
/// @param entry_idx Position in the Table
/// @return CHIP_NO_ERROR if removal was successful, errors if failed to remove the entry or to update the fabric after removing it
template <class StorageId, class StorageData, size_t kIteratorsMax>
CHIP_ERROR FabricTableImpl<StorageId, StorageData, kIteratorsMax>::RemoveTableEntryAtPosition(EndpointId endpoint,
                                                                                              FabricIndex fabric_index,
                                                                                              EntryIndex entry_idx)
{
    using TypedFabricEntryData = FabricEntryData<StorageId, StorageData, Serializer::kEntryMaxBytes(),
                                                 Serializer::kFabricMaxBytes(), Serializer::kMaxPerFabric()>;

    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    TypedFabricEntryData fabric(endpoint, fabric_index, mMaxPerFabric, mMaxPerEndpoint);

    ReturnErrorOnFailure(fabric.Load(mStorage));
    StorageId entryId;
    CHIP_ERROR err = fabric.FindByIndex(*mStorage, entry_idx, entryId);
    VerifyOrReturnValue(CHIP_ERROR_NOT_FOUND != err, CHIP_NO_ERROR);
    ReturnErrorOnFailure(err);

    return fabric.RemoveEntry(*mStorage, entryId);
}

template <class StorageId, class StorageData, size_t kIteratorsMax>
CHIP_ERROR FabricTableImpl<StorageId, StorageData, kIteratorsMax>::RemoveFabric(FabricIndex fabric_index)
{
    using TypedFabricEntryData = FabricEntryData<StorageId, StorageData, Serializer::kEntryMaxBytes(),
                                                 Serializer::kFabricMaxBytes(), Serializer::kMaxPerFabric()>;

    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    for (uint16_t index = 0; index < emberAfEndpointCount(); index++)
    {
        if (!emberAfEndpointIndexIsEnabled(index))
        {
            continue;
        }
        EndpointId endpoint = emberAfEndpointFromIndex(index);
        TypedFabricEntryData fabric(endpoint, fabric_index);
        EntryIndex idx = 0;
        CHIP_ERROR err = fabric.Load(mStorage);
        VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);
        if (CHIP_ERROR_NOT_FOUND == err)
        {
            continue;
        }

        while (idx < mMaxPerFabric)
        {
            err = RemoveTableEntryAtPosition(endpoint, fabric_index, idx);
            VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);
            idx++;
        }

        // Remove fabric entries on endpoint
        ReturnErrorOnFailure(fabric.Delete(mStorage));
    }

    return CHIP_NO_ERROR;
}

template <class StorageId, class StorageData, size_t kIteratorsMax>
CHIP_ERROR FabricTableImpl<StorageId, StorageData, kIteratorsMax>::RemoveEndpoint()
{
    using TypedFabricEntryData = FabricEntryData<StorageId, StorageData, Serializer::kEntryMaxBytes(),
                                                 Serializer::kFabricMaxBytes(), Serializer::kMaxPerFabric()>;

    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    for (FabricIndex fabric_index = kMinValidFabricIndex; fabric_index < kMaxValidFabricIndex; fabric_index++)
    {
        TypedFabricEntryData fabric(mEndpointId, fabric_index);
        CHIP_ERROR err = fabric.Load(mStorage);
        VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);
        if (CHIP_ERROR_NOT_FOUND == err)
        {
            continue;
        }

        EntryIndex idx = 0;
        while (idx < mMaxPerFabric)
        {
            err = RemoveTableEntryAtPosition(mEndpointId, fabric_index, idx);
            VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);
            idx++;
        };

        // Remove fabric entries on endpoint
        ReturnErrorOnFailure(fabric.Delete(mStorage));
    }

    return CHIP_NO_ERROR;
}

template <class StorageId, class StorageData, size_t kIteratorsMax>
void FabricTableImpl<StorageId, StorageData, kIteratorsMax>::SetEndpoint(EndpointId endpoint)
{
    mEndpointId = endpoint;
}

template <class StorageId, class StorageData, size_t kIteratorsMax>
void FabricTableImpl<StorageId, StorageData, kIteratorsMax>::SetTableSize(uint16_t endpointTableSize, uint16_t maxPerFabric)
{
    // Verify the endpoint passed size respects the limits of the device configuration
    VerifyOrDie(Serializer::kMaxPerFabric() > 0);
    VerifyOrDie(Serializer::kMaxPerEndpoint() > 0);
    mMaxPerEndpoint = std::min(Serializer::kMaxPerEndpoint(), endpointTableSize);
    mMaxPerFabric   = std::min(endpointTableSize, std::min(Serializer::kMaxPerFabric(), maxPerFabric));
}

template <class StorageId, class StorageData, size_t kIteratorsMax>
FabricTableImpl<StorageId, StorageData, kIteratorsMax>::EntryIteratorImpl::EntryIteratorImpl(
    FabricTableImpl & provider, FabricIndex fabricIdx, EndpointId endpoint, uint16_t maxPerFabric, uint16_t maxPerEndpoint) :
    mProvider(provider),
    mFabric(fabricIdx), mEndpoint(endpoint), mMaxPerFabric(maxPerFabric), mMaxPerEndpoint(maxPerEndpoint)
{
    using TypedFabricEntryData = FabricEntryData<StorageId, StorageData, Serializer::kEntryMaxBytes(),
                                                 Serializer::kFabricMaxBytes(), Serializer::kMaxPerFabric()>;

    TypedFabricEntryData fabric(mEndpoint, fabricIdx, mMaxPerFabric, mMaxPerEndpoint);
    ReturnOnFailure(fabric.Load(provider.mStorage));
    mTotalEntries = fabric.entry_count;
    mEntryIndex   = 0;
}

template <class StorageId, class StorageData, size_t kIteratorsMax>
size_t FabricTableImpl<StorageId, StorageData, kIteratorsMax>::EntryIteratorImpl::Count()
{
    return mTotalEntries;
}

template <class StorageId, class StorageData, size_t kIteratorsMax>
bool FabricTableImpl<StorageId, StorageData, kIteratorsMax>::EntryIteratorImpl::Next(TableEntry & output)
{
    using TypedFabricEntryData = FabricEntryData<StorageId, StorageData, Serializer::kEntryMaxBytes(),
                                                 Serializer::kFabricMaxBytes(), Serializer::kMaxPerFabric()>;

    TypedFabricEntryData fabric(mEndpoint, mFabric);

    VerifyOrReturnError(fabric.Load(mProvider.mStorage) == CHIP_NO_ERROR, false);

    // looks for next available entry
    PersistentStore<Serializer::kEntryMaxBytes()> persistentStore;
    while (mEntryIndex < mMaxPerFabric)
    {
        if (fabric.entry_map[mEntryIndex].IsValid())
        {
            TableEntryData<StorageId, StorageData> entry(mEndpoint, mFabric, output.mStorageId, output.mStorageData, mEntryIndex);
            VerifyOrReturnError(persistentStore.Load(entry, mProvider.mStorage) == CHIP_NO_ERROR, false);
            mEntryIndex++;

            return true;
        }

        mEntryIndex++;
    }

    return false;
}

template <class StorageId, class StorageData, size_t kIteratorsMax>
void FabricTableImpl<StorageId, StorageData, kIteratorsMax>::EntryIteratorImpl::Release()
{}
} // namespace Storage
} // namespace app
} // namespace chip
