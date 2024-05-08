/**
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
#include <access/Privilege.h>
#include <lib/core/DataModelTypes.h>

// Privilege mocks here are MUCH more strict so that
// testing code can generally validatate access without something
// being permissive like kView.

chip::Access::Privilege MatterGetAccessPrivilegeForReadAttribute(chip::ClusterId cluster, chip::AttributeId attribute)
{
    return chip::Access::Privilege::kAdminister;
}

chip::Access::Privilege MatterGetAccessPrivilegeForWriteAttribute(chip::ClusterId cluster, chip::AttributeId attribute)
{
    return chip::Access::Privilege::kAdminister;
}

chip::Access::Privilege MatterGetAccessPrivilegeForInvokeCommand(chip::ClusterId cluster, chip::CommandId command)
{
    return chip::Access::Privilege::kAdminister;
}

chip::Access::Privilege MatterGetAccessPrivilegeForReadEvent(chip::ClusterId cluster, chip::EventId event)
{
    return chip::Access::Privilege::kAdminister;
}
