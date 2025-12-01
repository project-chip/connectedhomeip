/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "MockPowerTopologyDelegate.h"

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PowerTopology;

CHIP_ERROR MockPowerTopologyDelegate::GetAvailableEndpointAtIndex(size_t index, EndpointId & endpointId)
{
    if (mAvailableEndpointsError != CHIP_NO_ERROR)
    {
        return mAvailableEndpointsError;
    }

    if (index >= mAvailableEndpoints.size())
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    endpointId = mAvailableEndpoints[index];
    return CHIP_NO_ERROR;
}

CHIP_ERROR MockPowerTopologyDelegate::GetActiveEndpointAtIndex(size_t index, EndpointId & endpointId)
{
    if (mActiveEndpointsError != CHIP_NO_ERROR)
    {
        return mActiveEndpointsError;
    }

    if (index >= mActiveEndpoints.size())
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    endpointId = mActiveEndpoints[index];
    return CHIP_NO_ERROR;
}
