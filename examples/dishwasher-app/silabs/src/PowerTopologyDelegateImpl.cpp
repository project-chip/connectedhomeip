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

#include <app/util/af-types.h>
#include <app/util/endpoint-config-api.h>
#include <lib/core/CHIPError.h>

#include "PowerTopologyDelegateImpl.h"

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PowerTopology;

CHIP_ERROR PowerTopologyDelegateImpl::GetAvailableEndpointAtIndex(size_t index, EndpointId & endpointId)
{
    VerifyOrReturnError(index < MATTER_ARRAY_SIZE(mAvailableEndpointIdList), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);

    endpointId = mAvailableEndpointIdList[index];

    return (endpointId == kInvalidEndpointId) ? CHIP_ERROR_INVALID_ARGUMENT : CHIP_NO_ERROR;
}

CHIP_ERROR PowerTopologyDelegateImpl::GetActiveEndpointAtIndex(size_t index, EndpointId & endpointId)
{
    VerifyOrReturnError(index < MATTER_ARRAY_SIZE(mActiveEndpointIdList), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);

    endpointId = mActiveEndpointIdList[index];

    return (endpointId == kInvalidEndpointId) ? CHIP_ERROR_INVALID_ARGUMENT : CHIP_NO_ERROR;
}

CHIP_ERROR PowerTopologyDelegateImpl::AddActiveEndpoint(EndpointId endpointId)
{
    bool AvailableEndpointFlag   = false;
    bool ActiveEndpointFlag      = false;
    uint16_t availableIndexToAdd = MAX_ENDPOINT_COUNT;

    for (uint16_t index = 0; index < MAX_ENDPOINT_COUNT; index++)
    {
        if (mAvailableEndpointIdList[index] == endpointId)
        {
            AvailableEndpointFlag = true;
        }
        if (mActiveEndpointIdList[index] == endpointId)
        {
            ActiveEndpointFlag = true;
        }
        if (mActiveEndpointIdList[index] == kInvalidEndpointId)
        {
            availableIndexToAdd = index;
        }
    }

    // Availability and duplication check
    VerifyOrReturnError(AvailableEndpointFlag && !ActiveEndpointFlag && (availableIndexToAdd != MAX_ENDPOINT_COUNT),
                        CHIP_ERROR_INVALID_ARGUMENT);

    mActiveEndpointIdList[availableIndexToAdd] = endpointId;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerTopologyDelegateImpl::RemoveActiveEndpoint(EndpointId endpointId)
{
    for (uint16_t index = 0; index < MAX_ENDPOINT_COUNT; index++)
    {
        if (mActiveEndpointIdList[index] == endpointId)
        {
            mActiveEndpointIdList[index] = kInvalidEndpointId;
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_INVALID_ARGUMENT;
}

void PowerTopologyDelegateImpl::InitAvailableEndpointList()
{
    for (uint16_t index = 0; index < MAX_ENDPOINT_COUNT; index++)
    {
        mAvailableEndpointIdList[index] =
            (emberAfEndpointIndexIsEnabled(index)) ? emberAfEndpointFromIndex(index) : kInvalidEndpointId;

        mActiveEndpointIdList[index] = kInvalidEndpointId;
    }
}
