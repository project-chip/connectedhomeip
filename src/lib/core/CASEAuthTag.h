/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

#include <array>

#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/core/NodeId.h>
#include <lib/support/CodeUtils.h>

namespace chip {

typedef uint32_t CASEAuthTag;

static constexpr CASEAuthTag kUndefinedCAT    = 0;
static constexpr NodeId kTagIdentifierMask    = 0x0000'0000'FFFF'0000ULL;
static constexpr uint32_t kTagIdentifierShift = 16;
static constexpr NodeId kTagVersionMask       = 0x0000'0000'0000'FFFFULL;

// Maximum number of CASE Authenticated Tags (CAT) in the CHIP certificate subject.
static constexpr size_t kMaxSubjectCATAttributeCount = CHIP_CONFIG_CERT_MAX_RDN_ATTRIBUTES - 2;

struct CATValues
{
    std::array<CASEAuthTag, kMaxSubjectCATAttributeCount> values = { kUndefinedCAT };

    /* @brief Returns maximum number of CAT values that the array can contain.
     */
    static constexpr size_t size() { return std::tuple_size<decltype(values)>::value; }

    /**
     * @return the number of CATs present in the set (values not equal to kUndefinedCAT)
     */
    size_t GetNumTagsPresent() const
    {
        size_t count = 0;
        for (auto cat : values)
        {
            count += (cat != kUndefinedCAT) ? 1 : 0;
        }
        return count;
    }

    /**
     * @return true if `tag` is in the set exactly, false otherwise.
     */
    bool Contains(CASEAuthTag tag) const
    {
        for (auto candidate : values)
        {
            if ((candidate != kUndefinedCAT) && (candidate == tag))
            {
                return true;
            }
        }

        return false;
    }

    /**
     * @brief Returns true if this set contains any version of the `identifier`
     *
     * @param identifier - CAT identifier to find
     * @return true if the identifier is in the set, false otherwise
     */
    bool ContainsIdentifier(uint16_t identifier) const
    {
        for (auto candidate : values)
        {
            uint16_t candidate_identifier = static_cast<uint16_t>((candidate & kTagIdentifierMask) >> kTagIdentifierShift);
            if ((candidate != kUndefinedCAT) && (identifier == candidate_identifier))
            {
                return true;
            }
        }

        return false;
    }

    /* @brief Returns true if subject input checks against one of the CATs in the values array.
     */
    bool CheckSubjectAgainstCATs(NodeId subject) const
    {
        VerifyOrReturnError(IsCASEAuthTag(subject), false);

        for (auto cat : values)
        {
            // All valid CAT values are always in the beginning of the array followed by kUndefinedCAT values.
            ReturnErrorCodeIf(cat == kUndefinedCAT, false);
            if (((cat & kTagIdentifierMask) == (subject & kTagIdentifierMask)) &&
                ((cat & kTagVersionMask) >= (subject & kTagVersionMask)))
            {
                return true;
            }
        }
        return false;
    }

    bool operator==(const CATValues & other) const
    {
        // Two sets of CATs confer equal permissions if the sets are exactly equal.
        // Ignoring kUndefinedCAT values, evaluate this.
        if (this->GetNumTagsPresent() != other.GetNumTagsPresent())
        {
            return false;
        }
        for (auto cat : this->values)
        {
            if (cat == kUndefinedCAT)
            {
                continue;
            }

            if (!other.Contains(cat))
            {
                return false;
            }
        }
        return true;
    }
    bool operator!=(const CATValues & other) const { return !(*this == other); }

    static constexpr size_t kSerializedLength = kMaxSubjectCATAttributeCount * sizeof(CASEAuthTag);
    typedef uint8_t Serialized[kSerializedLength];

    CHIP_ERROR Serialize(Serialized & outSerialized) const
    {
        uint8_t * p = outSerialized;
        for (size_t i = 0; i < kMaxSubjectCATAttributeCount; i++)
        {
            Encoding::LittleEndian::Write32(p, values[i]);
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Deserialize(const Serialized & inSerialized)
    {
        const uint8_t * p = inSerialized;
        for (size_t i = 0; i < kMaxSubjectCATAttributeCount; i++)
        {
            values[i] = Encoding::LittleEndian::Read32(p);
        }
        return CHIP_NO_ERROR;
    }
};

static constexpr CATValues kUndefinedCATs = { { kUndefinedCAT } };

constexpr NodeId NodeIdFromCASEAuthTag(CASEAuthTag aCAT)
{
    return kMinCASEAuthTag | aCAT;
}

constexpr CASEAuthTag CASEAuthTagFromNodeId(NodeId aNodeId)
{
    return aNodeId & kMaskCASEAuthTag;
}

constexpr bool IsValidCASEAuthTag(CASEAuthTag aCAT)
{
    return (aCAT & kTagVersionMask) > 0;
}

constexpr uint16_t GetCASEAuthTagIdentifier(CASEAuthTag aCAT)
{
    return static_cast<uint16_t>((aCAT & kTagIdentifierMask) >> kTagIdentifierShift);
}

constexpr uint16_t GetCASEAuthTagVersion(CASEAuthTag aCAT)
{
    return static_cast<uint16_t>(aCAT & kTagVersionMask);
}

} // namespace chip
