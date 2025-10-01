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
#include <app/clusters/zone-management-server/ZoneManagementCluster.h>
#include <app/static-cluster-config/ZoneManagement.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>

#include <cstdint>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ZoneManagement;

namespace {

static constexpr size_t kZoneManagementFixedClusterCount = ZoneManagement::StaticApplicationConfig::kFixedClusterConfig.size();
static constexpr size_t kZoneManagementMaxClusterCount =
    kZoneManagementFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<ZoneManagementCluster> gServers[kZoneManagementMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        gServers[clusterInstanceIndex].Create(endpointId);
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

void emberAfZoneManagementClusterServerInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId = endpointId,
            .clusterId = ZoneManagement::Id,
            .fixedClusterInstanceCount = kZoneManagementFixedClusterCount,
            .maxClusterInstanceCount = kZoneManagementMaxClusterCount,
            .fetchFeatureMap = false,
            .fetchOptionalAttributes = false,
        },
        integrationDelegate);
}

void MatterZoneManagementClusterServerShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId = endpointId,
            .clusterId = ZoneManagement::Id,
            .fixedClusterInstanceCount = kZoneManagementFixedClusterCount,
            .maxClusterInstanceCount = kZoneManagementMaxClusterCount,
        },
        integrationDelegate);
}
