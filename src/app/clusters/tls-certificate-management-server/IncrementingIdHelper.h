/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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

#include <lib/support/PersistentData.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Tls {

/** A helper which stores a mapping of (fabric, IDs) as well as the next available ID */
template <class StorageId, size_t NumEntries>
class IncrementingIdHelper
{
    struct StoredEntry
    {
        StorageId id;
        FabricIndex fabric = kUndefinedFabricIndex;
    };
    std::array<StoredEntry, NumEntries> mMapping;

public:
    uint16_t mNextId = 0;
    size_t mCount    = 0;

    void Clear()
    {
        mNextId = 0;

        for (auto & entry : mMapping)
        {
            entry.fabric = kUndefinedFabricIndex;
            entry.id.Clear();
        }
    }

    CHIP_ERROR SerializeMapping(TLV::TLVWriter & writer, TLV::Tag idTag, TLV::Tag fabricTag) const
    {
        for (size_t i = 0; i < mCount; i++)
        {
            const auto & stored = mMapping[i];
            if (stored.fabric == kUndefinedFabricIndex)
            {
                continue;
            }
            TLV::TLVType entryIdContainer;
            ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, entryIdContainer));
            ReturnErrorOnFailure(writer.Put(idTag, stored.id.Value()));
            ReturnErrorOnFailure(writer.Put(fabricTag, stored.fabric));
            ReturnErrorOnFailure(writer.EndContainer(entryIdContainer));
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR DeserializeMapping(TLV::TLVReader & reader, TLV::Tag idTag, TLV::Tag fabricTag)
    {
        size_t i = 0;
        CHIP_ERROR err;
        while ((err = reader.Next(TLV::AnonymousTag())) == CHIP_NO_ERROR)
        {
            TLV::TLVType entryIdContainer;
            if (i >= NumEntries)
            {
                // In-memory is not allowed to be smaller than what we've stored in persistence,
                // due to bindings to FabricTableImpl
                return CHIP_ERROR_INTERNAL;
            }
            auto & stored = mMapping[i];
            ReturnErrorOnFailure(reader.EnterContainer(entryIdContainer));
            ReturnErrorOnFailure(reader.Next(idTag));
            ReturnErrorOnFailure(reader.Get(stored.id.Value()));
            ReturnErrorOnFailure(reader.Next(fabricTag));
            ReturnErrorOnFailure(reader.Get(stored.fabric));
            ReturnErrorOnFailure(reader.ExitContainer(entryIdContainer));

            if (stored.fabric != kUndefinedFabricIndex)
            {
                ++i;
            }
        }
        mCount = i;
        VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
        return CHIP_NO_ERROR;
    }

    template <class Table, typename F>
    CHIP_ERROR Remove(Table & table, FabricIndex fabric, StorageId id, F save)
    {
        // Find the entry in the global mapping
        StoredEntry * foundEntry = nullptr;
        for (size_t i = 0; i < mCount; ++i)
        {
            auto & entry = mMapping[i];
            if (entry.fabric == fabric && entry.id == id)
            {
                foundEntry = &entry;
                break;
            }
        }
        if (foundEntry == nullptr)
        {
            return CHIP_ERROR_NOT_FOUND;
        }
        foundEntry->fabric = kUndefinedFabricIndex;
        foundEntry->id.Clear();
        ReturnErrorOnFailure(save());

        auto removeResult = table.RemoveTableEntry(fabric, id);
        if (removeResult == CHIP_ERROR_NOT_FOUND)
        {
            // Index is out of sync; keep it updated & return NOT_FOUND
            return CHIP_ERROR_NOT_FOUND;
        }

        if (removeResult != CHIP_NO_ERROR)
        {
            // Failed to remove, re-add
            foundEntry->fabric = fabric;
            foundEntry->id     = id;
            ReturnErrorOnFailure(save());
        }
        return removeResult;
    }

    template <typename F>
    CHIP_ERROR RemoveAll(FabricIndex fabric, F save)
    {
        // Find the entry in the global mapping
        uint16_t foundCount = 0;
        for (size_t i = 0; i < mCount; ++i)
        {
            auto & entry = mMapping[i];
            if (entry.fabric == fabric)
            {
                entry.fabric = kUndefinedFabricIndex;
                entry.id.Clear();
                ++foundCount;
            }
        }
        if (foundCount == 0)
        {
            return CHIP_ERROR_NOT_FOUND;
        }
        return save();
    }

    CHIP_ERROR GetNextId(uint16_t maxId)
    {
        VerifyOrReturnError(mCount < mMapping.size(), CHIP_ERROR_NO_MEMORY);

        bool looped = false;
        bool taken;
        uint16_t outOfIdCheck = mNextId;
        do
        {
            if (mNextId == outOfIdCheck)
            {
                VerifyOrReturnError(!looped, CHIP_ERROR_ENDPOINT_POOL_FULL);
                looped = true;
            }
            if (mNextId == maxId)
            {
                mNextId = 0;
            }
            taken = false;
            for (size_t i = 0; i < mCount; i++)
            {
                if (mMapping[i].id == mNextId)
                {
                    taken = true;
                    ++mNextId;
                    break;
                }
            }
        } while (taken);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ConsumeId(FabricIndex fabric, uint16_t & id)
    {
        id          = mNextId++;
        auto & used = mMapping[mCount++];
        used.id     = StorageId(id);
        used.fabric = fabric;
        return CHIP_NO_ERROR;
    }
};

} // namespace Tls
} // namespace Clusters

} // namespace app
} // namespace chip
