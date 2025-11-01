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

#include <app/clusters/general-commissioning-server/general-commissioning-cluster.h>
#include <app/static-cluster-config/GeneralCommissioning.h>
#include <data-model-providers/codegen/ClusterIntegration.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::GeneralCommissioning::Attributes;
using namespace chip::app::Clusters::GeneralCommissioning::StaticApplicationConfig;
using chip::Protocols::InteractionModel::Status;

namespace {

// GeneralCommissioningCluster implementation is specifically implemented
// only for the root endpoint (endpoint 0)
// So either:
//   - we have a fixed config and it is endpoint 0 OR
//   - we have a fully dynamic config

static constexpr size_t kGeneralCommissioningFixedClusterCount =
    GeneralCommissioning::StaticApplicationConfig::kFixedClusterConfig.size();

static_assert((kGeneralCommissioningFixedClusterCount == 0) ||
                  ((kGeneralCommissioningFixedClusterCount == 1) &&
                   GeneralCommissioning::StaticApplicationConfig::kFixedClusterConfig[0].endpointNumber == kRootEndpointId),
              "General Commissioning cluster MUST be on endpoint 0");

LazyRegisteredServerCluster<GeneralCommissioningCluster> gServer;

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {

        gServer.Create();

        // Configure optional attributes based on fetched bits
        gServer.Cluster().GetOptionalAttributes() = GeneralCommissioningCluster::OptionalAttributes(optionalAttributeBits);

        return gServer.Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gServer.IsConstructed(), nullptr);
        return &gServer.Cluster();
    }

    // Nothing to destroy: separate singleton class without constructor/destructor is used
    void ReleaseRegistration(unsigned clusterInstanceIndex) override {}
};

} // namespace

namespace chip::app::Clusters::GeneralCommissioning {

GeneralCommissioningCluster * Instance()
{
    VerifyOrReturnValue(gServer.IsConstructed(), nullptr);
    return &gServer.Cluster();
}

} // namespace chip::app::Clusters::GeneralCommissioning

void MatterGeneralCommissioningClusterInitCallback(EndpointId endpointId)
{
    VerifyOrReturn(endpointId == kRootEndpointId);
    IntegrationDelegate integrationDelegate;

    // register a singleton server (root endpoint only)
    // Startup() will be called automatically by the registry when context is set
    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = kRootEndpointId,
            .clusterId                 = GeneralCommissioning::Id,
            .fixedClusterInstanceCount = GeneralCommissioning::StaticApplicationConfig::kFixedClusterConfig.size(),
            .maxClusterInstanceCount   = 1, // Cluster is a singleton on the root node and this is the only thing supported
            .fetchFeatureMap           = false,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterGeneralCommissioningClusterShutdownCallback(EndpointId endpointId)
{
    VerifyOrReturn(endpointId == kRootEndpointId);

    IntegrationDelegate integrationDelegate;

    // Shutdown() will be called automatically by the registry when context is cleared
    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = kRootEndpointId,
            .clusterId                 = GeneralCommissioning::Id,
            .fixedClusterInstanceCount = GeneralCommissioning::StaticApplicationConfig::kFixedClusterConfig.size(),
            .maxClusterInstanceCount   = 1, // Cluster is a singleton on the root node and this is the only thing supported
        },
        integrationDelegate);
}

void MatterGeneralCommissioningPluginServerInitCallback() {}
void MatterGeneralCommissioningPluginServerShutdownCallback() {}
