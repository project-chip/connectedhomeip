/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/groups-server/GroupsCluster.h>
#include <app/static-cluster-config/Groups.h>
#include <app/util/config.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
#include <app/clusters/scenes-server/CodegenIntegration.h> // nogncheck
#endif

#ifdef ZCL_USING_IDENTIFY_CLUSTER_SERVER
#include <app/clusters/identify-server/CodegenIntegration.h> // nogncheck
#endif

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Groups;

namespace {

constexpr size_t kGroupsFixedClusterCount = Groups::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kGroupsMaxClusterCount   = kGroupsFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<GroupsCluster> gServers[kGroupsMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        Credentials::GroupDataProvider * groupDataProvider = Credentials::GetGroupDataProvider();
        VerifyOrDie(groupDataProvider != nullptr);

        gServers[clusterInstanceIndex].Create(endpointId,
                                              GroupsCluster::Context{
                                                  .groupDataProvider = *groupDataProvider,
#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
                                                  .scenesIntegration = ScenesManagement::FindClusterOnEndpoint(endpointId),
#else
                                                  .scenesIntegration   = nullptr,
#endif
#ifdef ZCL_USING_IDENTIFY_CLUSTER_SERVER
                                                  .identifyIntegration = FindIdentifyClusterOnEndpoint(endpointId),
#else
                                                  .identifyIntegration = nullptr,
#endif

                                              });
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

void MatterGroupsClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = Groups::Id,
            .fixedClusterInstanceCount = kGroupsFixedClusterCount,
            .maxClusterInstanceCount   = kGroupsMaxClusterCount,
            .fetchFeatureMap           = false,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterGroupsClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = Groups::Id,
            .fixedClusterInstanceCount = kGroupsFixedClusterCount,
            .maxClusterInstanceCount   = kGroupsMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

void MatterGroupsPluginServerInitCallback() {}
void MatterGroupsPluginServerShutdownCallback() {}
