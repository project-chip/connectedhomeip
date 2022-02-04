/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/core/CHIPConfig.h>
#include <lib/core/NodeId.h>
#include <lib/support/CodeUtils.h>

namespace chip {

typedef uint32_t CASEAuthTag;

static constexpr CASEAuthTag kUndefinedCAT = 0;
static constexpr NodeId kTagIdentifierMask = 0x0000'0000'FFFF'0000ULL;
static constexpr NodeId kTagVersionMask    = 0x0000'0000'0000'FFFFULL;

// Maximum number of CASE Authenticated Tags (CAT) in the CHIP certificate subject.
static constexpr size_t kMaxSubjectCATAttributeCount = CHIP_CONFIG_CERT_MAX_RDN_ATTRIBUTES - 2;

struct CATValues
{
    CASEAuthTag values[kMaxSubjectCATAttributeCount] = { kUndefinedCAT };

    /* @brief Returns size of the CAT values array.
     */
    static constexpr size_t size() { return ArraySize(values); }

    /* @brief Returns true if subject input checks against one of the CATs in the values array.
     */
    bool CheckSubjectAgainstCATs(NodeId subject) const
    {
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

} // namespace chip
