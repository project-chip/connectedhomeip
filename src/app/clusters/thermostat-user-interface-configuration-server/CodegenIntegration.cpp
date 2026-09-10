/*
 *
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

#include "CodegenIntegration.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/thermostat-user-interface-configuration-server/ThermostatUserInterfaceConfigurationCluster.h>
#include <app/static-cluster-config/ThermostatUserInterfaceConfiguration.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ThermostatUserInterfaceConfiguration;
using namespace chip::app::Clusters::ThermostatUserInterfaceConfiguration::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace {

constexpr size_t kFixedClusterCount = ThermostatUserInterfaceConfiguration::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kMaxClusterCount   = kFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<ThermostatUserInterfaceConfigurationCluster> gServers[kMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        ThermostatUserInterfaceConfigurationCluster::Config config;
        config.optionalAttributes = ThermostatUserInterfaceConfigurationCluster::OptionalAttributeSet(optionalAttributeBits);

        if (TemperatureDisplayMode::GetDefault(endpointId, &config.temperatureDisplayMode) != Status::Success)
        {
            config.temperatureDisplayMode = TemperatureDisplayModeEnum::kCelsius;
        }
        if (KeypadLockout::GetDefault(endpointId, &config.keypadLockout) != Status::Success)
        {
            config.keypadLockout = KeypadLockoutEnum::kNoLockout;
        }
        if (config.optionalAttributes.IsSet(ScheduleProgrammingVisibility::Id))
        {
            if (ScheduleProgrammingVisibility::GetDefault(endpointId, &config.scheduleProgrammingVisibility) != Status::Success)
            {
                config.scheduleProgrammingVisibility = ScheduleProgrammingVisibilityEnum::kScheduleProgrammingPermitted;
            }
        }

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

void MatterThermostatUserInterfaceConfigurationClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = ThermostatUserInterfaceConfiguration::Id,
            .fixedClusterInstanceCount = kFixedClusterCount,
            .maxClusterInstanceCount   = kMaxClusterCount,
            .fetchFeatureMap           = false,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterThermostatUserInterfaceConfigurationClusterShutdownCallback(EndpointId endpointId,
                                                                       MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = ThermostatUserInterfaceConfiguration::Id,
            .fixedClusterInstanceCount = kFixedClusterCount,
            .maxClusterInstanceCount   = kMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

namespace chip::app::Clusters::ThermostatUserInterfaceConfiguration {

ThermostatUserInterfaceConfigurationCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * cluster = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = ThermostatUserInterfaceConfiguration::Id,
            .fixedClusterInstanceCount = kFixedClusterCount,
            .maxClusterInstanceCount   = kMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<ThermostatUserInterfaceConfigurationCluster *>(cluster);
}

namespace Attributes {

namespace TemperatureDisplayMode {

Protocols::InteractionModel::Status Get(EndpointId endpoint,
                                        ThermostatUserInterfaceConfiguration::TemperatureDisplayModeEnum * value)
{
    auto * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }

    *value = cluster->GetTemperatureDisplayMode();
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, ThermostatUserInterfaceConfiguration::TemperatureDisplayModeEnum value)
{
    auto * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }

    return cluster->SetTemperatureDisplayMode(value);
}

} // namespace TemperatureDisplayMode

namespace KeypadLockout {

Protocols::InteractionModel::Status Get(EndpointId endpoint, ThermostatUserInterfaceConfiguration::KeypadLockoutEnum * value)
{
    auto * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }

    *value = cluster->GetKeypadLockout();
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, ThermostatUserInterfaceConfiguration::KeypadLockoutEnum value)
{
    auto * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }

    return cluster->SetKeypadLockout(value);
}

} // namespace KeypadLockout

namespace ScheduleProgrammingVisibility {

Protocols::InteractionModel::Status Get(EndpointId endpoint,
                                        ThermostatUserInterfaceConfiguration::ScheduleProgrammingVisibilityEnum * value)
{
    auto * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }

    *value = cluster->GetScheduleProgrammingVisibility();
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint,
                                        ThermostatUserInterfaceConfiguration::ScheduleProgrammingVisibilityEnum value)
{
    auto * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }

    return cluster->SetScheduleProgrammingVisibility(value);
}

} // namespace ScheduleProgrammingVisibility

} // namespace Attributes

} // namespace chip::app::Clusters::ThermostatUserInterfaceConfiguration
