/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "OOBAccessors.h"
#include "RootNode.h"
#include <lib/support/CodeUtils.h>
#include <oob-accessors/clusters/BasicInformationOOBAccessor.h>

namespace chip::app {

void RegisterOOBAccessors(RootNode & device, OOBAccessorRegistry & registry)
{
    LogErrorOnFailure(
        registry.Register(std::make_unique<BasicInformationOOBAccessor>(device.BasicInformation(), device.GetEndpointId())));
}

} // namespace chip::app
