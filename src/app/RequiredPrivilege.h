/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
