/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "ConcreteAttributePath.h"
#include "ConcreteCommandPath.h"
#include "ConcreteEventPath.h"

#include <app/util/privilege-storage.h>

#include <access/Privilege.h>

#include <lib/core/CHIPCore.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {

class RequiredPrivilege
{
    using Privilege = Access::Privilege;

    static constexpr Privilege kPrivilegeMapper[] = { Privilege::kView, Privilege::kOperate, Privilege::kManage,
                                                      Privilege::kAdminister };

    static_assert(ArraySize(kPrivilegeMapper) > kMatterAccessPrivilegeView &&
                      kPrivilegeMapper[kMatterAccessPrivilegeView] == Privilege::kView,
                  "Must map privilege correctly");
    static_assert(ArraySize(kPrivilegeMapper) > kMatterAccessPrivilegeOperate &&
                      kPrivilegeMapper[kMatterAccessPrivilegeOperate] == Privilege::kOperate,
                  "Must map privilege correctly");
    static_assert(ArraySize(kPrivilegeMapper) > kMatterAccessPrivilegeManage &&
                      kPrivilegeMapper[kMatterAccessPrivilegeManage] == Privilege::kManage,
                  "Must map privilege correctly");
    static_assert(ArraySize(kPrivilegeMapper) > kMatterAccessPrivilegeAdminister &&
                      kPrivilegeMapper[kMatterAccessPrivilegeAdminister] == Privilege::kAdminister,
                  "Must map privilege correctly");
    static_assert(ArraySize(kPrivilegeMapper) > kMatterAccessPrivilegeMaxValue, "Must map all privileges");

public:
    static Privilege ForReadAttribute(const ConcreteAttributePath & path)
    {
        return kPrivilegeMapper[MatterGetAccessPrivilegeForReadAttribute(path.mClusterId, path.mAttributeId)];
    }

    static Privilege ForWriteAttribute(const ConcreteAttributePath & path)
    {
        return kPrivilegeMapper[MatterGetAccessPrivilegeForWriteAttribute(path.mClusterId, path.mAttributeId)];
    }

    static Privilege ForInvokeCommand(const ConcreteCommandPath & path)
    {
        return kPrivilegeMapper[MatterGetAccessPrivilegeForInvokeCommand(path.mClusterId, path.mCommandId)];
    }

    static Privilege ForReadEvent(const ConcreteEventPath & path)
    {
        return kPrivilegeMapper[MatterGetAccessPrivilegeForReadEvent(path.mClusterId, path.mEventId)];
    }
};

} // namespace app
} // namespace chip
