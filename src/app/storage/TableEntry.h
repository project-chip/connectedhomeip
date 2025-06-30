/*
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

/**
 * This file contains common storage structures for FabricTableImpl
 */
#pragma once

#include <lib/support/CommonIterator.h>
#include <lib/support/TypeTraits.h>

namespace chip {
namespace app {
namespace Storage {
namespace Data {

// Storage index for entries in nvm
typedef uint16_t EntryIndex;
inline constexpr EntryIndex kUndefinedEntryIndex = 0xffff;

/// @brief Struct combining both ID and data of a table entry
template <class StorageId, class StorageData>
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

/// @brief Struct combining both ID and data of a table entry
template <class StorageId, class StorageData>
struct TableEntryRef
{
    // ID
    StorageId & mStorageId;

    // DATA
    StorageData & mStorageData;

    TableEntryRef(StorageId & id, StorageData & data) : mStorageId(id), mStorageData(data) {}
    TableEntryRef(TableEntry<StorageId, StorageData> & entry) : mStorageId(entry.mStorageId), mStorageData(entry.mStorageData) {}

    bool operator==(const TableEntryRef & other) const
    {
        return (mStorageId == other.mStorageId && mStorageData == other.mStorageData);
    }

    void operator=(const TableEntryRef & other)
    {
        mStorageId   = other.mStorageId;
        mStorageData = other.mStorageData;
    }

    void operator=(const TableEntry<StorageId, StorageData> & other)
    {
        mStorageId   = other.mStorageId;
        mStorageData = other.mStorageData;
    }
};

/// @brief a CommonIterator which allows processing of just the StorageData
template <class StorageId, class StorageData>
class TableEntryDataConvertingIterator : public CommonIterator<StorageData>
{
public:
    TableEntryDataConvertingIterator(CommonIterator<TableEntryRef<StorageId, StorageData>> & iterator) : mIterator(iterator) {}

    size_t Count() override { return mIterator.Count(); }

    bool Next(StorageData & item) override
    {
        StorageId id;
        TableEntryRef ref(id, item);
        return mIterator.Next(ref);
    }

    void Release() override { return mIterator.Release(); }

protected:
    CommonIterator<TableEntryRef<StorageId, StorageData>> & mIterator;
};
} // namespace Data
} // namespace Storage
} // namespace app
} // namespace chip
