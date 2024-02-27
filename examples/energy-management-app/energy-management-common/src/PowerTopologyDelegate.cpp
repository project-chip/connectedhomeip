/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <PowerTopologyDelegate.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PowerTopology;

CHIP_ERROR PowerTopologyDelegate::GetAvailableEndpointAtIndex(size_t index, EndpointId & endpointId)
{
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

CHIP_ERROR PowerTopologyDelegate::GetActiveEndpointAtIndex(size_t index, EndpointId & endpointId)
{
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

CHIP_ERROR PowerTopologyInstance::Init()
{
    return Instance::Init();
}

void PowerTopologyInstance::Shutdown()
{
    Instance::Shutdown();
}
