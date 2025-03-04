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

#include "chef-window-covering.h"
#include "app/clusters/window-covering-server/window-covering-server.h"
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <app/util/endpoint-config-api.h>

using namespace chip::app::Clusters;

constexpr size_t kWindowCoveringDelegateTableSize =
    MATTER_DM_WINDOW_COVERING_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
static_assert(kWindowCoveringDelegateTableSize <= kEmberInvalidEndpointIndex, "WindowCovering Delegate table size error");

std::unique_ptr<WindowCovering::ChefDelegate> gDelegateTable[kWindowCoveringDelegateTableSize];

std::unique_ptr<WindowCovering::ChefDelegate> * GetDelegate(chip::EndpointId endpoint)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, WindowCovering::Id, MATTER_DM_WINDOW_COVERING_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kWindowCoveringDelegateTableSize ? nullptr : gDelegateTable[ep]);
}

void InitChefWindowCoveringCluster()
{
    const uint16_t endpointCount = emberAfEndpointCount();

    for (uint16_t endpointIndex = 0; endpointIndex < endpointCount; endpointIndex++)
    {
        // Get endpoint ID from index.
        chip::EndpointId endpointId = emberAfEndpointFromIndex(endpointIndex);
        if (endpointId == chip::kInvalidEndpointId)
        {
            continue;
        }

        // Check if endpoint has WindowCovering cluster enabled
        uint16_t epIndex = emberAfGetClusterServerEndpointIndex(endpointId, WindowCovering::Id,
                                                                MATTER_DM_WINDOW_COVERING_CLUSTER_SERVER_ENDPOINT_COUNT);
        if (epIndex >= kWindowCoveringDelegateTableSize)
            continue;

        // Skip if delegate was already initialized.
        if (gDelegateTable[epIndex])
            continue;

        gDelegateTable[epIndex] = std::make_unique<WindowCovering::ChefDelegate>();
        gDelegateTable[epIndex]->SetEndpoint(endpointId);
        WindowCovering::SetDefaultDelegate(endpointId, gDelegateTable[epIndex].get());
    }
}
