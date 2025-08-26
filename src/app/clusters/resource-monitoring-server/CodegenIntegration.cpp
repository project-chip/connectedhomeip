/*
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
#include <app/clusters/resource-monitoring-server/resource-monitoring-cluster.h>
#include <app/static-cluster-config/ResourceMonitoringServer.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

#include <array>
#include <cstdint>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

static constexpr size_t kResourceMonitoringFixedClusterCount =
    ResourceMonitoring::StaticApplicationConfig::kFixedClusterConfig.size();
static constexpr size_t kResourceMonitoringMaxClusterCount = kResourceMonitoringFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<ResourceMonitoringCluster> gServers[kResourceMonitoringMaxClusterCount];

// Find the 0-based array index corresponding to the given endpoint id.
// Log an error if not found.
bool findEndpointWithLog(EndpointId endpointId, ClusterId clusterId, uint16_t & outArrayIndex)
{

    uint16_t arrayIndex =
        emberAfGetClusterServerEndpointIndex(endpointId, clusterId, kResourceMonitoringFixedClusterCount);

    if (arrayIndex >= kResourceMonitoringMaxClusterCount)
    {
        ChipLogError(AppServer, "Cound not find endpoint index for endpoint %u", endpointId);
        return false;
    }

    outArrayIndex = arrayIndex;

    return true;
}

} // namespace

void emberAfOtaSoftwareUpdateProviderClusterInitCallback(EndpointId endpointId, ClusterId clusterId)
{
    uint16_t arrayIndex = 0;
    if (!findEndpointWithLog(endpointId, clusterId, arrayIndex))
    {
        return;
    }
    gServers[arrayIndex].Create(endpointId);
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(gServers[arrayIndex].Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register Resource Monitogin on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
}

void emberAfOtaSoftwareUpdateProviderClusterShutdownCallback(EndpointId endpointId)
{
    uint16_t arrayIndex = 0;
    if (!findEndpointWithLog(endpointId, arrayIndex))
    {
        return;
    }

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&gServers[arrayIndex].Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to unregister Resource Monitoring on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
    gServers[arrayIndex].Destroy();
}

void MatterResourceMonitoringPluginServerInitCallback() {}

void MatterResourceMonitoringPluginServerShutdownCallback() {}