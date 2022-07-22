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

#include "RequiredPrivilege.h"

namespace chip {
namespace app {

constexpr Access::Privilege RequiredPrivilege::kPrivilegeMapper[];

} // namespace app
} // namespace chip

int __attribute__((weak)) MatterGetAccessPrivilegeForReadAttribute(chip::ClusterId cluster, chip::AttributeId attribute)
{
    return kMatterAccessPrivilegeAdminister;
}

int __attribute__((weak)) MatterGetAccessPrivilegeForWriteAttribute(chip::ClusterId cluster, chip::AttributeId attribute)
{
    return kMatterAccessPrivilegeAdminister;
}

int __attribute__((weak)) MatterGetAccessPrivilegeForInvokeCommand(chip::ClusterId cluster, chip::CommandId command)
{
    return kMatterAccessPrivilegeAdminister;
}

int __attribute__((weak)) MatterGetAccessPrivilegeForReadEvent(chip::ClusterId cluster, chip::EventId event)
{
    return kMatterAccessPrivilegeAdminister;
}
