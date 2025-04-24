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
 * @brief Implementation of a storage in nonvolatile storage of a templatized table that stores by fabric index.
 *
 * FabricTableImpl is an implementation that allows to store arbitrary entities using PersistentStorageDelegate.
 * It handles the storage of entities by their StorageId and EnpointID over multiple fabrics.
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
    CHIP_ERROR GetEndpointEntryCount(uint8_t & entry_count);
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

    void SetEndpoint(EndpointId endpoint);
    void SetTableSize(uint16_t endpointEntryTableSize, uint16_t maxPerFabric);
    bool IsInitialized() { return (mStorage != nullptr); }

protected:
    // gcc bug prevents us from using a static variable; see:
    // https://stackoverflow.com/questions/50638053/constexpr-static-data-member-without-initializer
    static constexpr EndpointId kEntryEndpointClusterID();

    // This constructor is meant for test purposes, it allows to change the defined max for entries per fabric and global, which
    // allows to simulate OTA where this value was changed
    FabricTableImpl(uint16_t maxEntriesPerFabric, uint16_t maxEntriesPerEndpoint) :
        mMaxPerFabric(maxEntriesPerFabric), mMaxPerEndpoint(maxEntriesPerEndpoint)
    {}

    // Global entry count
    CHIP_ERROR SetEndpointEntryCount(const uint8_t & entry_count);

    // wrapper function around emberAfGetClustersFromEndpoint to allow override when testing
    virtual uint8_t GetClustersFromEndpoint(ClusterId * clusterList, uint8_t listLen);

    // wrapper function around emberAfGetClusterCountForEndpoint to allow override when testing
    virtual uint8_t GetClusterCountFromEndpoint();

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
    EndpointId mEndpointId                     = kInvalidEndpointId;
    chip::PersistentStorageDelegate * mStorage = nullptr;
}; // class FabricTableImpl

template <class StorageId, class StorageData>
class DefaultSerializer
{
public:
    // gcc bug prevents us from using a static variable; see:
    // https://stackoverflow.com/questions/50638053/constexpr-static-data-member-without-initializer
    static constexpr size_t kPersistentStorageDataBufferMax();
    static constexpr size_t kPersistentFabricBufferMax();
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
    static StorageKeyName FabricEntryDataKey(FabricIndex fabric, EndpointId endpoint);
    static StorageKeyName FabricEntryKey(FabricIndex fabric, EndpointId endpoint, uint16_t idx);
}; // class DefaultSerializer

} // namespace Storage
} // namespace app
} // namespace chip
