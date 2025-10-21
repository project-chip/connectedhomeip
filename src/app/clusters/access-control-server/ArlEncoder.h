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

#include <access/AccessRestrictionProvider.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <credentials/FabricTable.h>

namespace chip {
namespace app {

/**
 * This class provides facilities for converting between access restriction
 * entries (as used by the system module) and access restriction entries (as used
 * by the generated cluster code).
 */
class ArlEncoder
{
public:
    ArlEncoder()  = default;
    ~ArlEncoder() = default;

    static CHIP_ERROR Convert(Clusters::AccessControl::AccessRestrictionTypeEnum from,
                              Access::AccessRestrictionProvider::Type & to);

    static CHIP_ERROR Convert(Access::AccessRestrictionProvider::Type from,
                              Clusters::AccessControl::AccessRestrictionTypeEnum & to);

    /**
     * Used for encoding commissionable access restriction entries.
     *
     * Typically used temporarily on the stack to encode:
     * - source: system level access restriction entry
     * - staging: generated cluster level code
     */
    class CommissioningEncodableEntry
    {
        using Entry              = Access::AccessRestrictionProvider::Entry;
        using StagingEntry       = Clusters::AccessControl::Structs::CommissioningAccessRestrictionEntryStruct::Type;
        using StagingRestriction = Clusters::AccessControl::Structs::AccessRestrictionStruct::Type;

    public:
        CommissioningEncodableEntry(const Entry & entry) : mEntry(entry) {}

        /**
         * Encode the constructor-provided entry into the TLV writer.
         */
        CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const;

        static constexpr bool kIsFabricScoped = false;

    private:
        CHIP_ERROR Stage() const;

        Entry mEntry;
        mutable StagingEntry mStagingEntry;
        mutable StagingRestriction mStagingRestrictions[CHIP_CONFIG_ACCESS_RESTRICTION_MAX_RESTRICTIONS_PER_ENTRY];
    };

    /**
     * Used for encoding access restriction entries.
     *
     * Typically used temporarily on the stack to encode:
     * - source: system level access restriction entry
     * - staging: generated cluster level code
     */
    class EncodableEntry
    {
        using Entry              = Access::AccessRestrictionProvider::Entry;
        using StagingEntry       = Clusters::AccessControl::Structs::AccessRestrictionEntryStruct::Type;
        using StagingRestriction = Clusters::AccessControl::Structs::AccessRestrictionStruct::Type;

    public:
        EncodableEntry(const Entry & entry) : mEntry(entry) {}

        /**
         * Encode the constructor-provided entry into the TLV writer.
         */
        CHIP_ERROR EncodeForRead(TLV::TLVWriter & writer, TLV::Tag tag, FabricIndex fabric) const;

        FabricIndex GetFabricIndex() const { return mEntry.fabricIndex; }

        static constexpr bool kIsFabricScoped = true;

    private:
        /**
         * Constructor-provided entry is staged into a staging entry.
         */
        CHIP_ERROR Stage() const;

        Entry mEntry;
        mutable StagingEntry mStagingEntry;
        mutable StagingRestriction mStagingRestrictions[CHIP_CONFIG_ACCESS_RESTRICTION_MAX_RESTRICTIONS_PER_ENTRY];
    };
};

} // namespace app
} // namespace chip
