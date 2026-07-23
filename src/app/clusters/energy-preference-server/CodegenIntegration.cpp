/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <app/static-cluster-config/EnergyPreference.h>
#include <data-model-providers/codegen/ClusterIntegration.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyPreference;
using namespace chip::app::Clusters::EnergyPreference::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace {

constexpr size_t kEnergyPreferenceFixedClusterCount =
    EnergyPreference::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kEnergyPreferenceMaxClusterCount =
    kEnergyPreferenceFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<EnergyPreferenceCluster> gServers[kEnergyPreferenceMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        BitFlags<Feature> features(featureMap);

        gServers[clusterInstanceIndex].Create(endpointId, features);

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

void MatterEnergyPreferencePluginServerInitCallback() {}

void MatterEnergyPreferenceClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = EnergyPreference::Id,
            .fixedClusterInstanceCount = kEnergyPreferenceFixedClusterCount,
            .maxClusterInstanceCount   = kEnergyPreferenceMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterEnergyPreferenceClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = EnergyPreference::Id,
            .fixedClusterInstanceCount = kEnergyPreferenceFixedClusterCount,
            .maxClusterInstanceCount   = kEnergyPreferenceMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

namespace chip::app::Clusters::EnergyPreference {

void SetDelegate(Delegate * aDelegate)
{
    EnergyPreferenceCluster::SetDelegate(aDelegate);
}

Delegate * GetDelegate()
{
    return EnergyPreferenceCluster::GetDelegate();
}

EnergyPreferenceCluster * FindClusterOnEndpoint(EndpointId endpoint)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * serverCluster = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpoint,
            .clusterId                 = EnergyPreference::Id,
            .fixedClusterInstanceCount = kEnergyPreferenceFixedClusterCount,
            .maxClusterInstanceCount   = kEnergyPreferenceMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<EnergyPreferenceCluster *>(serverCluster);
}

} // namespace chip::app::Clusters::EnergyPreference
