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

#include "CodegenIntegration.h"
#include <app/clusters/boolean-state-server/boolean-state-cluster.h>
#include <app/static-cluster-config/BooleanState.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BooleanState;
using namespace chip::app::Clusters::BooleanState::Attributes;

namespace {

constexpr size_t kBooleanStateFixedClusterCount = BooleanState::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kBooleanStateMaxClusterCount   = kBooleanStateFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<BooleanStateCluster> gServers[kBooleanStateFixedClusterCount];

// Find the 0-based array index corresponding to the given endpoint id.
// Log an error if not found.
bool FindEndpointWithLog(EndpointId endpointId, uint16_t & outArrayIndex)
{
    uint16_t arrayIndex = emberAfGetClusterServerEndpointIndex(endpointId, BooleanState::Id, kBooleanStateFixedClusterCount);

    if (arrayIndex >= kBooleanStateMaxClusterCount)
    {
        ChipLogError(AppServer, "Could not find endpoint index for endpoint %u", endpointId);
        return false;
    }
    outArrayIndex = arrayIndex;

    return true;
}

} // namespace

void emberAfBooleanStateClusterServerInitCallback(EndpointId endpointId)
{
    uint16_t arrayIndex = 0;
    VerifyOrReturn(FindEndpointWithLog(endpointId, arrayIndex));

    gServers[arrayIndex].Create(endpointId);
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(gServers[arrayIndex].Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register BooleanState cluster on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }
}

void MatterBooleanStateClusterServerShutdownCallback(EndpointId endpointId)
{
    uint16_t arrayIndex = 0;
    VerifyOrReturn(FindEndpointWithLog(endpointId, arrayIndex));

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&gServers[arrayIndex].Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to unregister BooleanState cluster on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }
    gServers[arrayIndex].Destroy();
}

namespace chip::app::Clusters::BooleanState {

BooleanStateCluster * GetClusterForEndpointIndex(EndpointId endpointId)
{
    uint16_t arrayIndex = 0;
    VerifyOrReturnError(FindEndpointWithLog(endpointId, arrayIndex), nullptr);

    return &gServers[arrayIndex].Cluster();
}

} // namespace chip::app::Clusters::BooleanState
