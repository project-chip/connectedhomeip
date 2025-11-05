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
#include <app/clusters/boolean-state-configuration-server/boolean-state-configuration-cluster.h>
#include <app/static-cluster-config/BooleanStateConfiguration.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <data-model-providers/codegen/ClusterIntegration.h>

#include <algorithm>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BooleanStateConfiguration;
using namespace chip::app::Clusters::BooleanStateConfiguration::Attributes;
using namespace chip::Protocols::InteractionModel;
using namespace chip::app::Clusters::BooleanStateConfiguration::Attributes;
namespace {

constexpr size_t kBooleanStateConfigurationFixedClusterCount =
    BooleanStateConfiguration::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kBooleanStateConfigurationMaxClusterCount =
    kBooleanStateConfigurationFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<BooleanStateConfigurationCluster> gServers[kBooleanStateConfigurationMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        uint8_t supportedSensitivityLevels{};
        if (SupportedSensitivityLevels::Get(endpointId, &supportedSensitivityLevels) != Status::Success)
        {
            supportedSensitivityLevels = BooleanStateConfigurationCluster::kMinSupportedSensitivityLevels;
        }
        uint8_t defaultSensitivityLevel{};
        if (DefaultSensitivityLevel::Get(endpointId, &defaultSensitivityLevel) != Status::Success)
        {
            defaultSensitivityLevel = supportedSensitivityLevels - 1;
        }

        BooleanStateConfigurationCluster::AlarmModeBitMask alarmsSupported{};
        if (AlarmsSupported::Get(endpointId, &alarmsSupported) != Status::Success)
        {
            alarmsSupported.ClearAll();
        }

        gServers[clusterInstanceIndex].Create(endpointId, BitMask<BooleanStateConfiguration::Feature>(featureMap),
                                              BooleanStateConfigurationCluster::OptionalAttributesSet(optionalAttributeBits),
                                              BooleanStateConfigurationCluster::StartupConfiguration{
                                                  .supportedSensitivityLevels = supportedSensitivityLevels,
                                                  .defaultSensitivityLevel    = defaultSensitivityLevel,
                                                  .alarmsSupported            = alarmsSupported,
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

void MatterBooleanStateConfigurationClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = BooleanStateConfiguration::Id,
            .fixedClusterInstanceCount = kBooleanStateConfigurationFixedClusterCount,
            .maxClusterInstanceCount   = kBooleanStateConfigurationMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterBooleanStateConfigurationClusterShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = BooleanStateConfiguration::Id,
            .fixedClusterInstanceCount = kBooleanStateConfigurationFixedClusterCount,
            .maxClusterInstanceCount   = kBooleanStateConfigurationMaxClusterCount,
        },
        integrationDelegate);
}

void MatterBooleanStateConfigurationPluginServerInitCallback() {}

namespace chip::app::Clusters::BooleanStateConfiguration {

BooleanStateConfigurationCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * booleanState = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = BooleanStateConfiguration::Id,
            .fixedClusterInstanceCount = kBooleanStateConfigurationFixedClusterCount,
            .maxClusterInstanceCount   = kBooleanStateConfigurationMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<BooleanStateConfigurationCluster *>(booleanState);
}
} // namespace chip::app::Clusters::BooleanStateConfiguration
