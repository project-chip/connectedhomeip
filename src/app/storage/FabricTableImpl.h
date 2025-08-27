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

#include <app/storage/TableEntry.h>
#include <lib/support/CommonIterator.h>
#include <lib/support/PersistentData.h>
#include <lib/support/TypeTraits.h>

namespace chip {
namespace app {
namespace Storage {

/**
 * @brief Container which exposes various compile-time constants for specializations
 * of FabricTableImpl.
 */
template <class StorageId, class StorageData>
class DefaultSerializer
{
public:
    // gcc bug prevents us from using a static variable; see:
    // https://stackoverflow.com/questions/50638053/constexpr-static-data-member-without-initializer
    // The number of bytes used by an entry (StorageData) + its metadata when persisting to storage
    static constexpr size_t kEntryMaxBytes();
    // The number of bytes used by FabricEntryData, which is dependent on the size of StorageId
    static constexpr size_t kFabricMaxBytes();
    // The max number of entries per fabric; this value directly affects memory usage
    static constexpr uint16_t kMaxPerFabric();
    // The max number of entries for the endpoint (programmatic limit)
    static constexpr uint16_t kMaxPerEndpoint();

    DefaultSerializer() {}
    ~DefaultSerializer(){};

    static CHIP_ERROR SerializeId(TLV::TLVWriter & writer, const StorageId & id);
    static CHIP_ERROR DeserializeId(TLV::TLVReader & reader, StorageId & id);

    static CHIP_ERROR SerializeData(TLV::TLVWriter & writer, const StorageData & data);
    static CHIP_ERROR DeserializeData(TLV::TLVReader & reader, StorageData & data);

    static StorageKeyName EndpointEntryCountKey(EndpointId endpoint);
    // The key for persisting the data for a fabric
    static StorageKeyName FabricEntryDataKey(FabricIndex fabric, EndpointId endpoint);
    // The key for persisting the data for an entry in a fabric; FabricEntryDataKey should be a root prefix
    // of this key, such that removing a fabric removes all its entries
    static StorageKeyName FabricEntryKey(FabricIndex fabric, EndpointId endpoint, uint16_t idx);

    // Clears the data to default values
    static void Clear(StorageData & data) { data.Clear(); }
}; // class DefaultSerializer

/**
 * @brief Implementation of a storage accessor in nonvolatile storage of a templatized table that stores by fabric index.
 *        This class does not actually hold the entries, but rather acts as a wrapper/accessor around the storage layer,
 *        reading entries from the storage pointed to by calling SetEndpoint.
 *
 * FabricTableImpl is an implementation that allows to store arbitrary entities using PersistentStorageDelegate.
 * It handles the storage of entities by their StorageId and EnpointId over multiple fabrics.
 */
template <class StorageId, class StorageData>
class FabricTableImpl
{
    using TableEntry = Data::TableEntryRef<StorageId, StorageData>;

public:
    using EntryIterator = CommonIterator<TableEntry>;
    using EntryIndex    = Data::EntryIndex;
    using Serializer    = DefaultSerializer<StorageId, StorageData>;

    virtual ~FabricTableImpl() { Finish(); };

    CHIP_ERROR Init(PersistentStorageDelegate & storage);
    void Finish();

    // Entry count
    /**
     * @brief Get the total number of stored entries for the entire endpoint
     * @param entry_count[out] the count of entries
     * @return CHIP_ERROR, CHIP_NO_ERROR if successful or if the Fabric was not found, specific CHIP_ERROR otherwise
     */
    CHIP_ERROR GetEndpointEntryCount(uint8_t & entry_count);

    /**
     * @brief Get the total number of stored entries for the specified fabric on the currently selected endpoint.
     * @param fabric_index the fabric to get the count for
     * @param entry_count[out] the count of entries
     * @return CHIP_ERROR, CHIP_NO_ERROR if successful or if the Fabric was not found, specific CHIP_ERROR otherwise
     */
    CHIP_ERROR GetFabricEntryCount(FabricIndex fabric_index, uint8_t & entry_count);

    // Data
    CHIP_ERROR GetRemainingCapacity(FabricIndex fabric_index, uint8_t & capacity);

    /**
     * @brief Writes the entry to persistent storage.
     * @param fabric_index the fabric to write the entry to
     * @param entry_id the unique entry identifier
     * @param data the source data
     * @param writeBuffer the buffer that will be used to write the data before being persisted; PersistentStorageDelegate does not
     * offer a way to stream bytes to be written
     */
    template <size_t kEntryMaxBytes>
    CHIP_ERROR SetTableEntry(FabricIndex fabric_index, const StorageId & entry_id, const StorageData & data,
                             PersistentStore<kEntryMaxBytes> & writeBuffer);

    /**
     * @brief Loads the entry from persistent storage.
     * @param fabric_index the fabric to load the entry from
     * @param entry_id the unique entry identifier
     * @param data the target for the loaded data
     * @param buffer the buffer that will be used to load from persistence; some data types in the data argument, such as
     * DecodableList, point directly into the buffer, and as such for those types of structures the lifetime of the buffer needs to
     * be equal to or greater than data
     */
    template <size_t kEntryMaxBytes>
    CHIP_ERROR GetTableEntry(FabricIndex fabric_index, StorageId & entry_id, StorageData & data,
                             PersistentStore<kEntryMaxBytes> & buffer);
    CHIP_ERROR FindTableEntry(FabricIndex fabric_index, const StorageId & entry_id, EntryIndex & idx);
    CHIP_ERROR RemoveTableEntry(FabricIndex fabric_index, const StorageId & entry_id);
    CHIP_ERROR RemoveTableEntryAtPosition(EndpointId endpoint, FabricIndex fabric_index, EntryIndex entry_idx);

    // Fabrics
    CHIP_ERROR RemoveFabric(FabricIndex fabric_index);
    CHIP_ERROR RemoveEndpoint();

    /**
     * @brief Selects the endpoint that the table will point to & entries will be read from.
     * @param endpoint the endpoint which entries will be stored to or read from.
     */
    void SetEndpoint(EndpointId endpoint);
    void SetTableSize(uint16_t endpointEntryTableSize, uint16_t maxPerFabric);
    bool IsInitialized() { return (mStorage != nullptr); }

    /**
     * @brief Iterates through all entries in fabric, calling iterateFn with the allocated iterator.
     * @tparam kEntryMaxBytes size of the buffer for loading entries, should match DefaultSerializer::kEntryMaxBytes
     * @tparam UnaryFunc a function of type std::function<CHIP_ERROR(EntryIterator & iterator)>; template arg for GCC inlining
     * efficiency
     * @param fabric the fabric to iterate entries for
     * @param store the in-memory buffer that an entry will be read into
     * @param iterateFn a function that will be called with the iterator; if this function returns an error result, iteration stops
     * and IterateEntries returns that same error result.
     */
    template <size_t kEntryMaxBytes, class UnaryFunc>
    CHIP_ERROR IterateEntries(FabricIndex fabric, PersistentStore<kEntryMaxBytes> & store, UnaryFunc iterateFn);

protected:
    // This constructor is meant for test purposes, it allows to change the defined max for entries per fabric and global, which
    // allows to simulate OTA where this value was changed
    FabricTableImpl(uint16_t maxEntriesPerFabric, uint16_t maxEntriesPerEndpoint) :
        mMaxPerFabric(maxEntriesPerFabric), mMaxPerEndpoint(maxEntriesPerEndpoint)
    {}

    // Endpoint entry count
    CHIP_ERROR SetEndpointEntryCount(const uint8_t & entry_count);

    /**
     * @brief Implementation of an iterator over the elements in the FabricTableImpl.
     *
     * If you would like to expose iterators in your subclass of FabricTableImpl, you can:
     * A) Use this class in an ObjectPool<EntryIteratorImpl> field to allow callers to obtain an iterator, with AutoRelease to free
     * resources B) Use IterateEntries to allocate on stack
     */
    template <size_t kEntryMaxBytes>
    class EntryIteratorImpl : public EntryIterator
    {
    public:
        EntryIteratorImpl(FabricTableImpl & provider, FabricIndex fabricIdx, EndpointId endpoint, uint16_t maxEntriesPerFabric,
                          uint16_t maxEntriesPerEndpoint, PersistentStore<kEntryMaxBytes> & store);
        size_t Count() override;
        bool Next(TableEntry & output) override;
        void Release() override;

    protected:
        FabricTableImpl & mProvider;
        PersistentStore<kEntryMaxBytes> & mStore;
        FabricIndex mFabric  = kUndefinedFabricIndex;
        EndpointId mEndpoint = kInvalidEndpointId;
        EntryIndex mNextEntryIdx;
        EntryIndex mEntryIndex = 0;
        uint8_t mTotalEntries  = 0;
        uint16_t mMaxPerFabric;
        uint16_t mMaxPerEndpoint;
    };

    uint16_t mMaxPerFabric;
    uint16_t mMaxPerEndpoint;
    EndpointId mEndpointId               = kInvalidEndpointId;
    PersistentStorageDelegate * mStorage = nullptr;
}; // class FabricTableImpl

} // namespace Storage
} // namespace app
} // namespace chip
