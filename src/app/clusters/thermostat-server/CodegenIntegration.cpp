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

#include <app-common/zap-generated/callback.h>
#include <app/clusters/thermostat-server/CodegenIntegration.h>
#include <lib/core/CHIPEncoding.h>

#include "ThermostatCluster.h"
#include <app/static-cluster-config/Thermostat.h>
#include <app/util/attribute-storage.h>
#include <app/util/attribute-table.h>
#include <app/util/endpoint-config-api.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <data-model-providers/codegen/CodegenProcessingConfig.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

constexpr size_t kThermostatFixedClusterCount = Thermostat::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kThermostatEndpointCount     = kThermostatFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

LazyRegisteredServerCluster<ThermostatCluster> gClusters[kThermostatEndpointCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {

        BitFlags<Thermostat::Feature> features(featureMap);

        using namespace chip::app::Clusters::Thermostat::Attributes;

        ThermostatCluster::OptionalAttributes optionalAttributes;

        optionalAttributes.AbsMinHeatSetpointLimit = features.Has(Thermostat::Feature::kHeating) &&
            emberAfContainsAttribute(endpointId, Thermostat::Id, AbsMinHeatSetpointLimit::Id);
        optionalAttributes.AbsMaxHeatSetpointLimit = features.Has(Thermostat::Feature::kHeating) &&
            emberAfContainsAttribute(endpointId, Thermostat::Id, AbsMaxHeatSetpointLimit::Id);
        optionalAttributes.AbsMinCoolSetpointLimit = features.Has(Thermostat::Feature::kCooling) &&
            emberAfContainsAttribute(endpointId, Thermostat::Id, AbsMinCoolSetpointLimit::Id);
        optionalAttributes.AbsMaxCoolSetpointLimit = features.Has(Thermostat::Feature::kCooling) &&
            emberAfContainsAttribute(endpointId, Thermostat::Id, AbsMaxCoolSetpointLimit::Id);

        optionalAttributes.LocalTemperatureCalibration = !features.Has(Thermostat::Feature::kLocalTemperatureNotExposed) &&
            emberAfContainsAttribute(endpointId, Thermostat::Id, LocalTemperatureCalibration::Id);
        optionalAttributes.MinHeatSetpointLimit = features.Has(Thermostat::Feature::kHeating) &&
            emberAfContainsAttribute(endpointId, Thermostat::Id, MinHeatSetpointLimit::Id);
        optionalAttributes.MaxHeatSetpointLimit = features.Has(Thermostat::Feature::kHeating) &&
            emberAfContainsAttribute(endpointId, Thermostat::Id, MaxHeatSetpointLimit::Id);
        optionalAttributes.MinCoolSetpointLimit = features.Has(Thermostat::Feature::kCooling) &&
            emberAfContainsAttribute(endpointId, Thermostat::Id, MinCoolSetpointLimit::Id);
        optionalAttributes.MaxCoolSetpointLimit = features.Has(Thermostat::Feature::kCooling) &&
            emberAfContainsAttribute(endpointId, Thermostat::Id, MaxCoolSetpointLimit::Id);
        optionalAttributes.RemoteSensing         = emberAfContainsAttribute(endpointId, Thermostat::Id, RemoteSensing::Id);
        optionalAttributes.ThermostatRunningMode = features.Has(Thermostat::Feature::kAutoMode) &&
            emberAfContainsAttribute(endpointId, Thermostat::Id, ThermostatRunningMode::Id);
        optionalAttributes.TemperatureSetpointHold =
            emberAfContainsAttribute(endpointId, Thermostat::Id, TemperatureSetpointHold::Id);
        optionalAttributes.TemperatureSetpointHoldDuration =
            emberAfContainsAttribute(endpointId, Thermostat::Id, TemperatureSetpointHoldDuration::Id);
        optionalAttributes.ThermostatRunningState =
            emberAfContainsAttribute(endpointId, Thermostat::Id, ThermostatRunningState::Id);
        optionalAttributes.SetpointChangeSource = emberAfContainsAttribute(endpointId, Thermostat::Id, SetpointChangeSource::Id);
        optionalAttributes.SetpointChangeAmount = emberAfContainsAttribute(endpointId, Thermostat::Id, SetpointChangeAmount::Id);
        optionalAttributes.SetpointChangeSourceTimestamp =
            emberAfContainsAttribute(endpointId, Thermostat::Id, SetpointChangeSourceTimestamp::Id);
        optionalAttributes.SetpointHoldExpiryTimestamp =
            emberAfContainsAttribute(endpointId, Thermostat::Id, SetpointHoldExpiryTimestamp::Id);
        optionalAttributes.OutdoorTemperature = emberAfContainsAttribute(endpointId, Thermostat::Id, OutdoorTemperature::Id);

        ChipLogError(Zcl, "Creating thermostat cluster for endpoint %d", endpointId);
        gClusters[clusterInstanceIndex].Create(endpointId, BitFlags<Thermostat::Feature>(featureMap), optionalAttributes);
        return gClusters[clusterInstanceIndex].Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gClusters[clusterInstanceIndex].IsConstructed(), nullptr);
        return &gClusters[clusterInstanceIndex].Cluster();
    }
    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gClusters[clusterInstanceIndex].Destroy(); }
};

Protocols::InteractionModel::Status SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
        return Protocols::InteractionModel::Status::Failure;
    }
    cluster->SetDelegate(delegate);
    return Protocols::InteractionModel::Status::Success;
}

ThermostatCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * thermostat = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = Thermostat::Id,
            .fixedClusterInstanceCount = kThermostatFixedClusterCount,
            .maxClusterInstanceCount   = kThermostatEndpointCount,
        },
        integrationDelegate);

    return static_cast<ThermostatCluster *>(thermostat);
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterThermostatClusterInitCallback(EndpointId endpointId)
{

    chip::app::Clusters::Thermostat::IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = Thermostat::Id,
            .fixedClusterInstanceCount = kThermostatFixedClusterCount,
            .maxClusterInstanceCount   = kThermostatEndpointCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterThermostatPluginServerInitCallback() {}

void MatterThermostatClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType clusterShutdownType)
{
    chip::app::Clusters::Thermostat::IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = Thermostat::Id,
            .fixedClusterInstanceCount = kThermostatFixedClusterCount,
            .maxClusterInstanceCount   = kThermostatEndpointCount,
        },
        integrationDelegate, clusterShutdownType);
}

void MatterThermostatClusterServerShutdownCallback(EndpointId endpointId) {}
