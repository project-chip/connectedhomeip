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

#include <app-common/zap-generated/callback.h>
#include <lib/core/CHIPEncoding.h>

#include "ThermostatCluster.h"
#include <app/util/attribute-storage-null-handling.h>
#include <app/util/attribute-storage.h>
#include <app/util/attribute-table.h>
#include <app/util/endpoint-config-api.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <data-model-providers/codegen/CodegenProcessingConfig.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

static constexpr size_t kThermostatEndpointCount =
    MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

LazyRegisteredServerCluster<ThermostatCluster> gClusters[kThermostatEndpointCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        ChipLogError(Zcl, "Creating thermostat cluster for endpoint %d", endpointId);
        gClusters[clusterInstanceIndex].Create(endpointId, BitFlags<Thermostat::Feature>(featureMap));
        return gClusters[clusterInstanceIndex].Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gClusters[clusterInstanceIndex].IsConstructed(), nullptr);
        return &gClusters[clusterInstanceIndex].Cluster();
    }
    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gClusters[clusterInstanceIndex].Destroy(); }
};

ThermostatCluster * ClusterForEndpoint(EndpointId endpointId)
{
    ThermostatCluster * cluster = nullptr;
    for (size_t i = 0; i < kThermostatEndpointCount; i++)
    {
        if (gClusters[i].IsConstructed())
        {
            if (gClusters[i].Cluster().Endpoint() == endpointId)
            {
                cluster = &gClusters[i].Cluster();
                break;
            }
        }
    }
    return cluster;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterThermostatClusterInitCallback(EndpointId endpointId)
{

    chip::app::Clusters::Thermostat::IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = Thermostat::Id,
            .fixedClusterInstanceCount = MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT,
            .maxClusterInstanceCount   = kThermostatEndpointCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterThermostatPluginServerInitCallback() {}

void MatterThermostatClusterShutdownCallback(EndpointId endpointId)
{
    chip::app::Clusters::Thermostat::IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = Thermostat::Id,
            .fixedClusterInstanceCount = MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT,
            .maxClusterInstanceCount   = kThermostatEndpointCount,
        },
        integrationDelegate);
}

void MatterThermostatClusterServerShutdownCallback(EndpointId endpointId) {}
