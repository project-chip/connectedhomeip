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

/**
 *    @file
 *          Contains a class handling creation of linked list of stored persistent data.
 */
#pragma once

#include <lib/core/DataModelTypes.h>
#include <lib/support/PersistentData.h>

namespace chip {
namespace CommonPersistentData {

inline constexpr uint8_t kdefaultUndefinedEntry = 0;

/// @brief Generic class to implement storage of a list persistently
/// @tparam EntryType : Type of entry depends on the stored data
/// @tparam kMaxSerializedSize : inherited from PersistentData class
template <typename EntryType, size_t kMaxSerializedSize>
struct StoredDataList : public PersistentData<kMaxSerializedSize>
{
    static constexpr TLV::Tag TagFirstEntry() { return TLV::ContextTag(1); }
    static constexpr TLV::Tag TagEntryCount() { return TLV::ContextTag(2); }

    EntryType first_entry = kdefaultUndefinedEntry;
    uint16_t entry_count  = 0;

    StoredDataList() = default;
    StoredDataList(EntryType first) : first_entry(first), entry_count(1) {}

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));

        ReturnErrorOnFailure(writer.Put(TagFirstEntry(), static_cast<uint16_t>(first_entry)));
        ReturnErrorOnFailure(writer.Put(TagEntryCount(), static_cast<uint16_t>(entry_count)));

        return writer.EndContainer(container);
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
        VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_INTERNAL);

        TLV::TLVType container;
        ReturnErrorOnFailure(reader.EnterContainer(container));

        // first_entry
        ReturnErrorOnFailure(reader.Next(TagFirstEntry()));
        ReturnErrorOnFailure(reader.Get(first_entry));
        // entry_count
        ReturnErrorOnFailure(reader.Next(TagEntryCount()));
        ReturnErrorOnFailure(reader.Get(entry_count));

        return reader.ExitContainer(container);
    }
};

inline constexpr size_t kPersistentFabricBufferMax = 32;
struct FabricList : StoredDataList<FabricIndex, kPersistentFabricBufferMax>
{
    // Subclasses need to define UpdateKey to be whatever fabric list key they
    // care about.

    void Clear() override
    {
        first_entry = kUndefinedFabricIndex;
        entry_count = 0;
    }
};
} // namespace CommonPersistentData
} // namespace chip
