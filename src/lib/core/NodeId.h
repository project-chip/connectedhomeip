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

#include <cstdint>

namespace chip {

// TODO: Consider making this a class and the various utility methods static
// methods.
using NodeId = uint64_t;

constexpr NodeId kUndefinedNodeId = 0ULL;

// The range of possible NodeId values has some pieces carved out for special
// uses.
constexpr NodeId kMinGroupNodeId = 0xFFFF'FFFF'FFFF'0000ULL;
// The max group id is complicated, depending on how we want to count the
// various special group ids.  Let's not define it for now, until we have use
// cases.

constexpr NodeId kMinTemporaryLocalId = 0xFFFF'FFFE'0000'0000ULL;
// We use the largest available temporary local id to represent
// kPlaceholderNodeId, so the range is narrowed compared to the spec.
constexpr NodeId kMaxTemporaryLocalId = 0xFFFF'FFFE'FFFF'FFFEULL;
constexpr NodeId kPlaceholderNodeId   = 0xFFFF'FFFE'FFFF'FFFFULL;

constexpr NodeId kMinCASEAuthTag1 = 0xFFFF'FFFD'0000'0000ULL;
constexpr NodeId kMaxCASEAuthTag1 = 0xFFFF'FFFD'FFFF'FFFFULL;

constexpr NodeId kMinCASEAuthTag2 = 0xFFFF'FFFC'0000'0000ULL;
constexpr NodeId kMaxCASEAuthTag2 = 0xFFFF'FFFC'FFFF'FFFFULL;

constexpr NodeId kMinPAKEKeyId = 0xFFFF'FFFB'0000'0000ULL;
constexpr NodeId kMaxPAKEKeyId = 0xFFFF'FFFB'FFFF'FFFFULL;

// There are more reserved ranges here, not assigned to anything yet, going down
// all the way to 0xFFFF'FFF0'0000'0000ULL

constexpr NodeId kMaxOperationalNodeId = 0xFFFF'FFEF'FFFF'FFFFULL;

constexpr bool IsOperationalNodeId(NodeId aNodeId)
{
    return (aNodeId != kUndefinedNodeId) && (aNodeId <= kMaxOperationalNodeId);
}

} // namespace chip
