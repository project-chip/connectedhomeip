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
#include <app/clusters/administrator-commissioning-server/AdministratorCommissioningCluster.h>
#include <app/static-cluster-config/AdministratorCommissioning.h>
#include <data-model-providers/codegen/ClusterIntegration.h>

#include <app/util/config.h> // REQUIRED for ifdefs for commands

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AdministratorCommissioning::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace {

// AdministratorCommissioningCluster implementation is specifically implemented
// only for the root endpoint (endpoint 0)
// So either:
//   - we have a fixed config and it is endpoint 0 OR
//   - we have a fully dynamic config

static constexpr size_t kAdministratorCommissioningFixedClusterCount =
    AdministratorCommissioning::StaticApplicationConfig::kFixedClusterConfig.size();

static_assert((kAdministratorCommissioningFixedClusterCount == 0) ||
                  ((kAdministratorCommissioningFixedClusterCount == 1) &&
                   AdministratorCommissioning::StaticApplicationConfig::kFixedClusterConfig[0].endpointNumber == kRootEndpointId),
              "Fixed administrator commissioning MUST be on endpoint 0");

#ifdef ADMINISTRATOR_COMMISSIONING_ENABLE_OPEN_BASIC_COMMISSIONING_WINDOW_CMD
using ClusterImpl = AdministratorCommissioningWithBasicCommissioningWindowCluster;
#else
using ClusterImpl = AdministratorCommissioningCluster;
#endif

// Exactly one instance allocated: implementation here supports root-node functionality and does not support
// generic functionality (e.g. for multi fabric)
LazyRegisteredServerCluster<ClusterImpl> gServer;

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        gServer.Create(endpointId, BitFlags<AdministratorCommissioning::Feature>(featureMap));
        return gServer.Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gServer.IsConstructed(), nullptr);
        return &gServer.Cluster();
    }
    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gServer.Destroy(); }
};

} // namespace

void MatterAdministratorCommissioningClusterInitCallback(EndpointId endpointId)
{
    // The implementation of the server we use here is only for the RootNode (i.e. endpoint 0)
    // singleton. Other uses (e.g. fabric sync) will need their own implementations and would be added
    // separately.
    VerifyOrReturn(endpointId == kRootEndpointId);

    IntegrationDelegate integrationDelegate;

    // register a singleton server (root endpoint only)
    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = AdministratorCommissioning::Id,
            .fixedClusterInstanceCount = AdministratorCommissioning::StaticApplicationConfig::kFixedClusterConfig.size(),
            .maxClusterInstanceCount   = 1, // only root-node functionality supported by this implementation
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterAdministratorCommissioningClusterShutdownCallback(EndpointId endpointId)
{
    VerifyOrReturn(endpointId == kRootEndpointId);

    IntegrationDelegate integrationDelegate;

    // unregister a singleton server (root endpoint only)
    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = AdministratorCommissioning::Id,
            .fixedClusterInstanceCount = AdministratorCommissioning::StaticApplicationConfig::kFixedClusterConfig.size(),
            .maxClusterInstanceCount   = 1, // only root-node functionality supported by this implementation
        },
        integrationDelegate);
}

void MatterAdministratorCommissioningPluginServerInitCallback() {}
void MatterAdministratorCommissioningPluginServerShutdownCallback() {}
