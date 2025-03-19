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
namespace common {

/**
 * @brief Implementation of a storage in nonvolatile storage of a templatized table that stores by fabric id.
 *
 * DefaultSceneTableImpl is an implementation that allows to store scenes using PersistentStorageDelegate.
 * It handles the storage of scenes by their ID, GroupID and EnpointID over multiple fabrics.
 * It is meant to be used exclusively when the scene cluster is enable for at least one endpoint
 * on the device.
 */
template <class StorageId, class StorageData, size_t kIteratorsMax>
class FabricTableImpl : public virtual FabricTable<StorageId, StorageData>
{
    using Super         = FabricTable<StorageId, StorageData>;
    using TableEntry    = typename Super::TableEntry;
    using EntryIterator = typename Super::EntryIterator;

public:
    ~FabricTableImpl() { Finish(); };

    CHIP_ERROR Init(PersistentStorageDelegate * storage) override;
    void Finish() override;

    // Scene count
    CHIP_ERROR GetEndpointEntryCount(uint8_t & entry_count) override;
    CHIP_ERROR GetFabricEntryCount(FabricIndex fabric_index, uint8_t & entry_count) override;

    // Data
    CHIP_ERROR GetRemainingCapacity(FabricIndex fabric_index, uint8_t & capacity) override;
    CHIP_ERROR SetTableEntry(FabricIndex fabric_index, const TableEntry & entry) override;
    CHIP_ERROR GetTableEntry(FabricIndex fabric_index, StorageId entry_id, TableEntry & entry) override;
    CHIP_ERROR RemoveTableEntry(FabricIndex fabric_index, StorageId entry_id) override;
    CHIP_ERROR RemoveTableEntryAtPosition(EndpointId endpoint, FabricIndex fabric_index, EntryIndex entry_idx) override;

    // Fabrics
    CHIP_ERROR RemoveFabric(FabricIndex fabric_index) override;
    CHIP_ERROR RemoveEndpoint() override;

    // Iterators
    EntryIterator * IterateTableEntries(FabricIndex fabric_index) override;

    void SetEndpoint(EndpointId endpoint);
    void SetTableSize(uint16_t endpointEntryTableSize);
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
    ObjectPool<EntryIteratorImpl, kIteratorsMax> mEntryIterators;
}; // class FabricTableImpl

template <class StorageId, class StorageData>
class DefaultSerializer
{
public:
    // gcc bug prevents us from using a static variable; see:
    // https://stackoverflow.com/questions/50638053/constexpr-static-data-member-without-initializer
    static constexpr size_t kPersistentStorageDataBufferMax();
    static constexpr size_t kPersistentFabricBufferMax();
    static constexpr uint16_t kMaxPerFabric();
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

} // namespace common
} // namespace app
} // namespace chip
