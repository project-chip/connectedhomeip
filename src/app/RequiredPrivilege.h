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

#include <access/Privilege.h>

#include <lib/core/CHIPCore.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {

// This functionality is intended to come from Ember, but until Ember supports it,
// this class will provide a workable alternative.
class RequiredPrivilege
{
    using Privilege = Access::Privilege;

public:
    static Privilege ForReadAttribute(ClusterId cluster, EndpointId endpoint, AttributeId attribute);
    static Privilege ForWriteAttribute(ClusterId cluster, EndpointId endpoint, AttributeId attribute);
    static Privilege ForInvokeCommand(ClusterId cluster, EndpointId endpoint, CommandId command);
    static Privilege ForReadEvent(ClusterId cluster, EndpointId endpoint, EventId event);

    static CHIP_ERROR OverrideForReadAttribute(ClusterId cluster, EndpointId endpoint, AttributeId attribute, Privilege privilege);
    static CHIP_ERROR OverrideForWriteAttribute(ClusterId cluster, EndpointId endpoint, AttributeId attribute, Privilege privilege);
    static CHIP_ERROR OverrideForInvokeCommand(ClusterId cluster, EndpointId endpoint, CommandId command, Privilege privilege);
    static CHIP_ERROR OverrideForReadEvent(ClusterId cluster, EndpointId endpoint, EventId event, Privilege privilege);

    static CHIP_ERROR UnoverrideForReadAttribute(ClusterId cluster, EndpointId endpoint, AttributeId attribute);
    static CHIP_ERROR UnoverrideForWriteAttribute(ClusterId cluster, EndpointId endpoint, AttributeId attribute);
    static CHIP_ERROR UnoverrideForInvokeCommand(ClusterId cluster, EndpointId endpoint, CommandId command);
    static CHIP_ERROR UnoverrideForReadEvent(ClusterId cluster, EndpointId endpoint, EventId event);
};

} // namespace app
} // namespace chip
