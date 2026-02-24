/*
 *
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

#include <app/clusters/switch-server/CodegenIntegration.h>
#include <app/clusters/switch-server/SwitchCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <app/static-cluster-config/Switch.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Switch;
using namespace chip::app::Clusters::Switch::Attributes;
using namespace chip::Protocols::InteractionModel;

namespace {

constexpr size_t kSwitchFixedClusterCount = Switch::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kSwitchMaxClusterCount   = kSwitchFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<SwitchCluster> gServers[kSwitchMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        app::OptionalAttributeSet<MultiPressMax::Id> optionalAttributeSet(optionalAttributeBits);
        BitFlags<Feature> features(featureMap);

        uint8_t numberOfPositions{};
        VerifyOrDie(NumberOfPositions::Get(endpointId, &numberOfPositions) == Status::Success);

        uint8_t multiPressMax{};
        // Enforce a valid configuration from ember
        if (features.Has(Feature::kMomentarySwitchMultiPress))
        {
            VerifyOrDie(optionalAttributeSet.IsSet(MultiPressMax::Id));
            VerifyOrDie(MultiPressMax::Get(endpointId, &multiPressMax) == Status::Success);
        }

        gServers[clusterInstanceIndex].Create(endpointId, features,
                                              SwitchCluster::StartupConfiguration{
                                                  .numberOfPositions = numberOfPositions,
                                                  .multiPressMax     = multiPressMax,
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

void MatterSwitchClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = Switch::Id,
            .fixedClusterInstanceCount = kSwitchFixedClusterCount,
            .maxClusterInstanceCount   = kSwitchMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterSwitchClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = Switch::Id,
            .fixedClusterInstanceCount = kSwitchFixedClusterCount,
            .maxClusterInstanceCount   = kSwitchMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

namespace chip::app::Clusters::Switch {

SwitchCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * cluster = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = Switch::Id,
            .fixedClusterInstanceCount = kSwitchFixedClusterCount,
            .maxClusterInstanceCount   = kSwitchMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<SwitchCluster *>(cluster);
}

} // namespace chip::app::Clusters::Switch

void MatterSwitchPluginServerInitCallback() {}
