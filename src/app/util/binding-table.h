/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 * @file API declarations for a binding table.
 */

#pragma once

#include <app/util/af-types.h>
#include <app/util/config.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/TLV.h>
#include <lib/support/DefaultStorageKeyAllocator.h>

namespace chip {

class BindingTable
{
    friend class Iterator;

public:
    BindingTable();

    class Iterator
    {
        friend class BindingTable;

    public:
        EmberBindingTableEntry & operator*() { return mTable->mBindingTable[mIndex]; }

        const EmberBindingTableEntry & operator*() const { return mTable->mBindingTable[mIndex]; }

        EmberBindingTableEntry * operator->() { return &(mTable->mBindingTable[mIndex]); }

        const EmberBindingTableEntry * operator->() const { return &(mTable->mBindingTable[mIndex]); }

        Iterator operator++();

        bool operator==(const Iterator & rhs) const { return mIndex == rhs.mIndex; }

        bool operator!=(const Iterator & rhs) const { return mIndex != rhs.mIndex; }

        uint8_t GetIndex() const { return mIndex; }

    private:
        BindingTable * mTable;
        uint8_t mPrevIndex;
        uint8_t mIndex;
    };

    CHIP_ERROR Add(const EmberBindingTableEntry & entry);

    const EmberBindingTableEntry & GetAt(uint8_t index);

    // The iter will be moved to the next item in the table after calling RemoveAt.
    CHIP_ERROR RemoveAt(Iterator & iter);

    // Returns the number of active entries in the binding table.
    // *NOTE* The function does not return the capacity of the binding table.
    uint8_t Size() const { return mSize; }

    Iterator begin();

    Iterator end();

    void SetPersistentStorage(PersistentStorageDelegate * storage) { mStorage = storage; }

    CHIP_ERROR LoadFromStorage();

    static BindingTable & GetInstance() { return sInstance; }

private:
    static BindingTable sInstance;

    static constexpr uint32_t kStorageVersion  = 1;
    static constexpr uint8_t kEntryStorageSize = TLV::EstimateStructOverhead(
        sizeof(FabricIndex), sizeof(EndpointId), sizeof(ClusterId), sizeof(EndpointId), sizeof(NodeId), sizeof(uint8_t));
    static constexpr uint8_t kListInfoStorageSize = TLV::EstimateStructOverhead(sizeof(kStorageVersion), sizeof(uint8_t));

    static constexpr uint8_t kTagStorageVersion = 1;
    static constexpr uint8_t kTagHead           = 2;
    static constexpr uint8_t kTagFabricIndex    = 1;
    static constexpr uint8_t kTagLocalEndpoint  = 2;
    static constexpr uint8_t kTagCluster        = 3;
    static constexpr uint8_t kTagRemoteEndpoint = 4;
    static constexpr uint8_t kTagNodeId         = 5;
    static constexpr uint8_t kTagGroupId        = 6;
    static constexpr uint8_t kTagNextEntry      = 7;
    static constexpr uint8_t kNextNullIndex     = 255;

    uint8_t GetNextAvaiableIndex();

    CHIP_ERROR SaveEntryToStorage(uint8_t index, uint8_t nextIndex);
    CHIP_ERROR SaveListInfo(uint8_t head);

    CHIP_ERROR LoadEntryFromStorage(uint8_t index, uint8_t & nextIndex);

    EmberBindingTableEntry mBindingTable[MATTER_BINDING_TABLE_SIZE];
    uint8_t mNextIndex[MATTER_BINDING_TABLE_SIZE];

    uint8_t mHead = kNextNullIndex;
    uint8_t mTail = kNextNullIndex;
    uint8_t mSize = 0;

    PersistentStorageDelegate * mStorage;
};

} // namespace chip
