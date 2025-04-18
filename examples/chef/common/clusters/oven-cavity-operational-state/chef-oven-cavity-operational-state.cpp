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

#include "chef-oven-cavity-operational-state.h"
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <lib/support/logging/CHIPLogging.h>

#ifdef MATTER_DM_PLUGIN_OVEN_CAVITY_OPERATIONAL_STATE_SERVER

namespace chef {
namespace OvenCavityOperationalState {

constexpr size_t kOvenCavityOperationalStateTableSize = MATTER_DM_OVEN_CAVITY_OPERATIONAL_STATE_SERVER_ENDPOINT_COUNT;
static_assert(kOvenCavityOperationalStateTableSize <= kEmberInvalidEndpointIndex, "OvenCavityOperationalState table size error");

std::unique_ptr<Delegate> gDelegateTable[kOvenCavityOperationalStateTableSize];
std::unique_ptr<chip::app::Clusters::OvenCavityOperationalState::Instance> gInstanceTable[kOvenCavityOperationalStateTableSize];

/**
 * Initializes OvenCavityOperationalState cluster for the app (all endpoints).
 */
void InitChefOvenCavityOperationalStateCluster()
{
    const uint16_t endpointCount = emberAfEndpointCount();

    for (uint16_t endpointIndex = 0; endpointIndex < endpointCount; endpointIndex++)
    {
        EndpointId endpointId = emberAfEndpointFromIndex(endpointIndex);
        if (endpointId == kInvalidEndpointId)
        {
            continue;
        }

        // Check if endpoint has OvenCavityOperationalState cluster enabled
        uint16_t epIndex = emberAfGetClusterServerEndpointIndex(endpointId, chip::app::Clusters::OvenCavityOperationalState::Id,
                                                                MATTER_DM_OVEN_CAVITY_OPERATIONAL_STATE_SERVER_ENDPOINT_COUNT);
        if (epIndex >= kOvenCavityOperationalStateTableSize)
            continue;

        gDelegateTable[epIndex] = std::make_unique<Delegate>();

        gInstanceTable[epIndex] =
            std::make_unique<chip::app::Clusters::OvenCavityOperationalState::Instance>(gDelegateTable[epIndex].get(), endpointId);
        gInstanceTable[epIndex]->Init();

        ChipLogProgress(DeviceLayer, "Endpoint %d OvenCavityOperationalState Initialized.", endpointId);
    }
}

} // namespace OvenCavityOperationalState
} // namespace chef

#endif // MATTER_DM_PLUGIN_OVEN_CAVITY_OPERATIONAL_STATE_SERVER
