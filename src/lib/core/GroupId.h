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

using GroupId = uint16_t;

constexpr GroupId kUndefinedGroupId = 0;

constexpr GroupId kMinUniversalGroupId = 0xFF00;
constexpr GroupId kMaxUniversalGroupId = 0xFFFF;

constexpr GroupId kMinApplicationGroupId = 0x0001;
constexpr GroupId kMaxApplicationGroupId = 0xFEFF;

constexpr GroupId kAllNodes     = 0xFFFF;
constexpr GroupId kAllNonSleepy = 0xFFFE;
constexpr GroupId kAllProxies   = 0xFFFD;

constexpr GroupId kMinUniversalGroupIdReserved = 0xFF00;
constexpr GroupId kMaxUniversalGroupIdReserved = 0xFFFC;

constexpr bool IsOperationalGroupId(GroupId aGroupId)
{
    return (aGroupId != kUndefinedGroupId) &&
        ((aGroupId < kMinUniversalGroupIdReserved) || (aGroupId > kMaxUniversalGroupIdReserved));
}

constexpr bool IsApplicationGroupId(GroupId aGroupId)
{
    return (aGroupId >= kMinApplicationGroupId) && (aGroupId <= kMaxApplicationGroupId);
}

constexpr bool IsUniversalGroupId(GroupId aGroupId)
{
    return (aGroupId >= kMinUniversalGroupId);
}

constexpr bool IsValidGroupId(GroupId aGroupId)
{
    return aGroupId != kUndefinedGroupId;
}

} // namespace chip
