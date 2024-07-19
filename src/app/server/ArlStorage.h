/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <access/AccessRestriction.h>
#include <credentials/FabricTable.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>

#include <app-common/zap-generated/cluster-objects.h>

namespace chip {
namespace app {

/**
 * Storage specifically for access restriction entries, which correspond to
 * the ARL attribute of the access control cluster.
 *
 * An object of this class should be initialized directly after the access
 * control module is initialized, as it will populate entries in the system
 * module from storage, and also install a listener in the system module to
 * keep storage up to date as entries change.
 *
 * This class also provides facilities for converting between access restriction
 * entries (as used by the system module) and access restriction entries (as used
 * by the generated cluster code).
 */
class ArlStorage
{
public:
    /**
     * Used for decoding access restriction entries.
     *
     * Typically used temporarily on the stack to decode:
     * - source: TLV
     * - staging: generated cluster level code
     * - destination: system level access restriction entry
     */
    class DecodableEntry
    {
        using Entry        = Access::AccessRestriction::Entry;
        using StagingEntry = Clusters::AccessControl::Structs::AccessRestrictionEntryStruct::DecodableType;

    public:
        DecodableEntry() = default;

        /**
         * Reader decodes into a staging entry, which is then unstaged
         * into a member entry.
         */
        CHIP_ERROR Decode(TLV::TLVReader & reader);

        Entry & GetEntry() { return mEntry; }

        const Entry & GetEntry() const { return mEntry; }

    public:
        static constexpr bool kIsFabricScoped = true;

        void SetFabricIndex(FabricIndex fabricIndex) { mEntry.fabricIndex = fabricIndex; }

    private:
        Entry mEntry;

        StagingEntry mStagingEntry;
    };

    /**
     * Used for encoding access restriction entries.
     *
     * Typically used temporarily on the stack to encode:
     * - source: system level access restriction entry
     * - staging: generated cluster level code
     * - destination: TLV
     */
    class EncodableEntry
    {
        using Entry              = Access::AccessRestriction::Entry;
        using StagingEntry       = Clusters::AccessControl::Structs::AccessRestrictionEntryStruct::Type;
        using StagingRestriction = Clusters::AccessControl::Structs::AccessRestrictionStruct::Type;

    public:
        EncodableEntry(std::shared_ptr<Entry> entry) : mEntry(entry) {}

        /**
         * Constructor-provided entry is staged into a staging entry,
         * which is then encoded into a writer.
         */
        CHIP_ERROR EncodeForRead(TLV::TLVWriter & writer, TLV::Tag tag, FabricIndex fabric) const;

        /**
         * Constructor-provided entry is staged into a staging entry,
         * which is then encoded into a writer.
         */
        CHIP_ERROR EncodeForWrite(TLV::TLVWriter & writer, TLV::Tag tag) const;

        /**
         * Constructor-provided entry is staged into a staging entry.
         */
        CHIP_ERROR Stage() const;

        StagingEntry & GetStagingEntry() { return mStagingEntry; }

        const StagingEntry & GetStagingEntry() const { return mStagingEntry; }

    public:
        static constexpr bool kIsFabricScoped = true;

        FabricIndex GetFabricIndex() const { return mEntry->fabricIndex; }

    private:
        std::shared_ptr<Entry> mEntry;

        mutable StagingEntry mStagingEntry;
        mutable StagingRestriction mStagingRestrictions[CHIP_CONFIG_ACCESS_RESTRICTION_MAX_RESTRICTIONS_PER_ENTRY];
    };

    virtual ~ArlStorage() = default;

    /**
     * Initialize should be called after chip::Access::AccessControl is initialized.
     *
     * Implementations should take this opportunity to populate AccessControl with ARL entries
     * loaded from persistent storage. A half-open range of fabrics [first, last) is provided
     * so this can be done on a per-fabric basis.
     *
     * Implementations should also install an entry change listener on AccessControl to maintain
     * ARL entries in persistent storage as they are changed.
     */
    virtual CHIP_ERROR Init(PersistentStorageDelegate & persistentStorage, ConstFabricIterator first, ConstFabricIterator last) = 0;
};

} // namespace app
} // namespace chip
