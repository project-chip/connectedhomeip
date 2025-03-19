/*
 *    Copyright (c) 2023 Project CHIP Authors
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
 * This file contains backwards-compatibility enum name definitions.  This file
 * is meant to be included at the end of cluster-enums.h, after all the normal
 * enum definitions are available.
 */
#pragma once

#include <lib/support/CHIPMemString.h>
#include <lib/support/CommonIterator.h>
#include <lib/support/IntrusiveList.h>
#include <lib/support/PersistentData.h>
#include <lib/support/Span.h>
#include <lib/support/TypeTraits.h>

namespace chip {
namespace app {
namespace common {

// Storage index for entries in nvm
typedef uint16_t EntryIndex;
inline constexpr EntryIndex kUndefinedEntryIndex = 0xffff;

template <class StorageId, class StorageData>
class FabricTable
{
public:
    /// @brief Struct combining both ID and data of a table entry
    struct TableEntry
    {
        // ID
        StorageId mStorageId;

        // DATA
        StorageData mStorageData;

        TableEntry() = default;
        TableEntry(StorageId id) : mStorageId(id) {}
        TableEntry(const StorageId id, const StorageData data) : mStorageId(id), mStorageData(data) {}

        bool operator==(const TableEntry & other) const
        {
            return (mStorageId == other.mStorageId && mStorageData == other.mStorageData);
        }

        void operator=(const TableEntry & other)
        {
            mStorageId   = other.mStorageId;
            mStorageData = other.mStorageData;
        }
    };

    FabricTable(){};

    virtual ~FabricTable(){};

    // Not copyable
    FabricTable(const FabricTable &)             = delete;
    FabricTable & operator=(const FabricTable &) = delete;

    virtual CHIP_ERROR Init(PersistentStorageDelegate * storage) = 0;
    virtual void Finish()                                        = 0;

    // Global count
    virtual CHIP_ERROR GetEndpointEntryCount(uint8_t & entry_count)                         = 0;
    virtual CHIP_ERROR GetFabricEntryCount(FabricIndex fabric_index, uint8_t & entry_count) = 0;

    // Data
    virtual CHIP_ERROR GetRemainingCapacity(FabricIndex fabric_index, uint8_t & capacity)                              = 0;
    virtual CHIP_ERROR SetTableEntry(FabricIndex fabric_index, const TableEntry & entry)                               = 0;
    virtual CHIP_ERROR GetTableEntry(FabricIndex fabric_index, StorageId entry_id, TableEntry & entry)                 = 0;
    virtual CHIP_ERROR RemoveTableEntry(FabricIndex fabric_index, StorageId entry_id)                                  = 0;
    virtual CHIP_ERROR RemoveTableEntryAtPosition(EndpointId endpoint, FabricIndex fabric_index, EntryIndex entry_idx) = 0;

    // Fabrics

    /**
     * @brief Removes all entries associated with a fabric index and the stored TableEntry that maps them
     * @param fabric_index Fabric index to remove
     * @return CHIP_ERROR, CHIP_NO_ERROR if successful or if the Fabric was not found, specific CHIP_ERROR otherwise
     * @note This function is meant to be used after a fabric is removed from the device, the implementation MUST ensure that it
     * won't interact with the actual fabric table as it will be removed beforehand.
     */
    virtual CHIP_ERROR RemoveFabric(FabricIndex fabric_index) = 0;
    virtual CHIP_ERROR RemoveEndpoint()                       = 0;

    // Iterators
    using EntryIterator = CommonIterator<TableEntry>;

    virtual EntryIterator * IterateTableEntries(FabricIndex fabric_index) = 0;
};

} // namespace common
} // namespace app
} // namespace chip
