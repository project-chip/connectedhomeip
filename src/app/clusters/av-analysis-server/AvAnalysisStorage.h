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

#include <vector>
#include <app-common/zap-generated/cluster-objects.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AvAnalysis {

// Spec defined max values
constexpr int8_t kMaxSupportedAmbientContexts     = 50;
constexpr int8_t kMaxActiveAmbientContextTriggers = 50;
constexpr int8_t kMaxContextTriggers              = 50;

// Max size for a TLV encoded Semantic Tag, this is used in calculating the buffer size for Context Triggers
constexpr size_t kSemanticTagStructSerializedSize =
    TLV::EstimateStructOverhead(sizeof(uint16_t), sizeof(uint8_t), sizeof(uint8_t), static_cast<size_t>(64)); /* max label length */

/**
 * Helper Struct to provide memory backing for the stored contexts given that some attributes use
 * non-owning types (e.g. CharSpan, DataModel::List).
 */
struct AmbientContextStorage
{
    /**
     * Indicator for whether or not a label is present
     */
    enum class LabelState : uint8_t
    {
        kNoLabel      = 0x00,
        kNullLabel    = 0x01,
        kLabelPresent = 0x02
    };

private:
    Globals::Structs::SemanticTagStruct::Type mContext;
    std::string mLabel;
    Optional<DataModel::Nullable<std::vector<uint16_t>>> mZoneIDs;
    LabelState mLabelState;

public:
    virtual ~AmbientContextStorage() = default;
    AmbientContextStorage()
    {
        mZoneIDs.ClearValue();
        mLabelState = LabelState::kNoLabel;
    }

    // Accessors and Mutators
    //
    Globals::Structs::SemanticTagStruct::Type GetContext()
    {
        switch (mLabelState)
        {
        case LabelState::kLabelPresent:
            mContext.label = MakeOptional(DataModel::MakeNullable(CharSpan(mLabel.c_str(), mLabel.size())));
            break;

        case LabelState::kNullLabel:
            mContext.label = MakeOptional(DataModel::NullNullable);
            break;

        case LabelState::kNoLabel:
            mContext.label = {};
            break;
        }

        return mContext;
    }

    void SetContext(Globals::Structs::SemanticTagStruct::Type aContext)
    {
        mContext = aContext;
        if (aContext.label.HasValue())
        {
            if (!aContext.label.Value().IsNull())
            {
                mLabel      = std::string(aContext.label.Value().Value().begin(), aContext.label.Value().Value().end());
                mLabelState = LabelState::kLabelPresent;
            }
            else
            {
                mLabelState = LabelState::kNullLabel;
            }
        }
        else
        {
            mLabelState = LabelState::kNoLabel;
        }
    }

    Optional<DataModel::Nullable<std::vector<uint16_t>>> GetZoneIDs() const { return mZoneIDs; }
    Optional<DataModel::Nullable<DataModel::List<const uint16_t>>> GetZoneIDsForEncode() const
    {
        if (mZoneIDs.HasValue())
        {
            if (mZoneIDs.Value().IsNull())
            {
                return chip::MakeOptional(DataModel::NullNullable);
            }
            else
            {
                return MakeOptional(DataModel::MakeNullable(
                    DataModel::List<const uint16_t>(mZoneIDs.Value().Value().data(), mZoneIDs.Value().Value().size())));
            }
        }

        return {};
    }
    void SetZoneIDs(chip::Optional<DataModel::Nullable<std::vector<uint16_t>>> aZoneIDs) { mZoneIDs = aZoneIDs; }
};

} // namespace AvAnalysis
} // namespace Clusters
} // namespace app
} // namespace chip
