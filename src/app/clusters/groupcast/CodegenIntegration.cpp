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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/groupcast/GroupcastCluster.h>
#include <app/static-cluster-config/Groupcast.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <data-model-providers/codegen/ClusterIntegration.h>

using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Groupcast::Attributes;
using namespace chip::app::Clusters::Groupcast::StaticApplicationConfig;
using chip::Protocols::InteractionModel::Status;

namespace {

LazyRegisteredServerCluster<GroupcastCluster> gServer;

// Groupcast implementation is specifically implemented
// only for the root endpoint (endpoint 0)

static constexpr size_t kGroupcastFixedClusterCount = Groupcast::StaticApplicationConfig::kFixedClusterConfig.size();

static_assert((kGroupcastFixedClusterCount == 0) ||
                  ((kGroupcastFixedClusterCount == 1) &&
                   Groupcast::StaticApplicationConfig::kFixedClusterConfig[0].endpointNumber == chip::kRootEndpointId),
              "Groupcast cluster MUST be on endpoint 0");

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(chip::EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        // No optional attributes
        gServer.Create();
        return gServer.Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gServer.IsConstructed(), nullptr);
        return &gServer.Cluster();
    }

    // Nothing to destroy: separate singleton class without constructor/destructor is used
    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gServer.Destroy(); }
};

} // namespace

void MatterGroupcastClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == chip::kRootEndpointId);

    IntegrationDelegate integrationDelegate;

    // register a singleton server (root endpoint only)
    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = Groupcast::Id,
            .fixedClusterInstanceCount = Groupcast::StaticApplicationConfig::kFixedClusterConfig.size(),
            .maxClusterInstanceCount   = 1, // Cluster is a singleton on the root node and this is the only thing supported
            .fetchFeatureMap           = false,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterGroupcastClusterShutdownCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == chip::kRootEndpointId);

    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = Groupcast::Id,
            .fixedClusterInstanceCount = Groupcast::StaticApplicationConfig::kFixedClusterConfig.size(),
            .maxClusterInstanceCount   = 1, // Cluster is a singleton on the root node and this is the only thing supported
        },
        integrationDelegate);
}

void MatterGroupcastPluginServerInitCallback() {}
void MatterGroupcastPluginServerShutdownCallback() {}
