/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
