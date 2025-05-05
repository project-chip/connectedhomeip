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

#include <lib/support/TypeTraits.h>

namespace chip {
namespace app {
namespace Storage {

/**
 * @brief Implementation of a storage accessor in nonvolatile storage of a templatized table that stores by fabric index.
 *        This class does not actually hold the entries, but rather acts as a wrapper/accessor around the storage layer,
 *        reading entries from the storage pointed to by calling SetEndpoint.
 *
 * FabricTableImpl is an implementation that allows to store arbitrary entities using PersistentStorageDelegate.
 * It handles the storage of entities by their StorageId and EnpointId over multiple fabrics.
 */
template <class StorageId, class StorageData, size_t kIteratorsMax>
class FabricTableImpl
{
    using TableEntry    = Data::TableEntry<StorageId, StorageData>;
    using EntryIterator = CommonIterator<TableEntry>;
    using EntryIndex    = Data::EntryIndex;

public:
    virtual ~FabricTableImpl() { Finish(); };

    CHIP_ERROR Init(PersistentStorageDelegate * storage);
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
    CHIP_ERROR SetTableEntry(FabricIndex fabric_index, const TableEntry & entry);
    CHIP_ERROR GetTableEntry(FabricIndex fabric_index, StorageId entry_id, TableEntry & entry);
    CHIP_ERROR RemoveTableEntry(FabricIndex fabric_index, StorageId entry_id);
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
     * If you would like to expose iterators in your subclass of FabricTableImpl, use this class
     * in an ObjectPool<EntryIteratorImpl> field to allow callers to obtain an iterator.
     */
    class EntryIteratorImpl : public EntryIterator
    {
    public:
        EntryIteratorImpl(FabricTableImpl & provider, FabricIndex fabricIdx, EndpointId endpoint, uint16_t maxEntriesPerFabric,
                          uint16_t maxEntriesPerEndpoint);
        size_t Count() override;
        bool Next(TableEntry & output) override;
        void Release() override;

    protected:
        FabricTableImpl & mProvider;
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
}; // class DefaultSerializer

} // namespace Storage
} // namespace app
} // namespace chip
