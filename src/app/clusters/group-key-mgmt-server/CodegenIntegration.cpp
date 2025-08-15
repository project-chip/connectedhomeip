/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/group-key-mgmt-server/group-key-mgmt-cluster.h>
#include <app/static-cluster-config/GroupKeyManagement.h>
#include <app/util/config.h>
#include <data-model-providers/codegen/ClusterIntegration.h>

namespace {
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::GroupKeyManagement;

static_assert((GroupKeyManagement::StaticApplicationConfig::kFixedClusterConfig.size() == 1 &&
               GroupKeyManagement::StaticApplicationConfig::kFixedClusterConfig[0].endpointNumber == kRootEndpointId) ||
              GroupKeyManagement::StaticApplicationConfig::kFixedClusterConfig.size() == 0);

LazyRegisteredServerCluster<GroupKeyManagementCluster> gServer;

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned emberEndpointIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        gServer.Create();
        return gServer.Registration();
    }

    ServerClusterInterface & FindRegistration(unsigned emberEndpointIndex) override { return gServer.Cluster(); }

    // Nothing to destroy: separate singleton class without constructor/destructor is used
    void ReleaseRegistration(unsigned emberEndpointIndex) override { gServer.Destroy(); }
};

} // namespace

void emberAfGroupKeyManagementClusterServerInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                      = endpointId,
            .clusterId                       = GroupKeyManagement::Id,
            .fixedClusterServerEndpointCount = 1,
            .maxEndpointCount                = 1,
            .fetchFeatureMap                 = false,
            .fetchOptionalAttributes         = false,
        },
        integrationDelegate);
}

void MatterGroupKeyManagementClusterServerShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                      = endpointId,
            .clusterId                       = GroupKeyManagement::Id,
            .fixedClusterServerEndpointCount = 1,
            .maxEndpointCount                = 1,
        },
        integrationDelegate);
}

void MatterGroupKeyManagementPluginServerInitCallback() {}

void MatterGroupKeyManagementPluginServerShutdownCallback() {}
