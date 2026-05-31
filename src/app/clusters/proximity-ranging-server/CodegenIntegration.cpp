/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/proximity-ranging-server/ProximityRangingCluster.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <data-model-providers/codegen/ClusterIntegration.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ProximityRanging;

namespace {

constexpr uint16_t kProximityRangingFixedClusterCount = FIXED_ENDPOINT_COUNT;
constexpr uint16_t kProximityRangingMaxClusterCount =
    kProximityRangingFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<ProximityRangingCluster> gServers[kProximityRangingMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        ProximityRangingCluster::Config config;
        config.WithFeatures(BitMask<Feature>(featureMap));

        gServers[clusterInstanceIndex].Create(endpointId, config);
        return gServers[clusterInstanceIndex].Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gServers[clusterInstanceIndex].IsConstructed(), nullptr);
        return &gServers[clusterInstanceIndex].Cluster();
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gServers[clusterInstanceIndex].Destroy(); }
};

} // namespace

void MatterProximityRangingClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = ProximityRanging::Id,
            .fixedClusterInstanceCount = kProximityRangingFixedClusterCount,
            .maxClusterInstanceCount   = kProximityRangingMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterProximityRangingClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = ProximityRanging::Id,
            .fixedClusterInstanceCount = kProximityRangingFixedClusterCount,
            .maxClusterInstanceCount   = kProximityRangingMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

void MatterProximityRangingPluginServerInitCallback() {}
void MatterProximityRangingPluginServerShutdownCallback() {}

namespace chip::app::Clusters::ProximityRanging {

ProximityRangingCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * cluster = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = ProximityRanging::Id,
            .fixedClusterInstanceCount = kProximityRangingFixedClusterCount,
            .maxClusterInstanceCount   = kProximityRangingMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<ProximityRangingCluster *>(cluster);
}

} // namespace chip::app::Clusters::ProximityRanging
