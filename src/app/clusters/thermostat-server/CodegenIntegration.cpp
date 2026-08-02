/*
 *    Copyright (c) 2025-2026 Project CHIP Authors
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

#include <app/clusters/thermostat-server/CodegenIntegration.h>

#include <app/util/attribute-storage.h>
#include <app/util/attribute-table.h>
#include <app/util/endpoint-config-api.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

ThermostatCluster::OptionalAttributes BaseIntegrationDelegate::GetOptionalAttributes(EndpointId endpointId,
                                                                                     BitFlags<Thermostat::Feature> features)
{
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
    optionalAttributes.TemperatureSetpointHold = emberAfContainsAttribute(endpointId, Thermostat::Id, TemperatureSetpointHold::Id);
    optionalAttributes.TemperatureSetpointHoldDuration =
        emberAfContainsAttribute(endpointId, Thermostat::Id, TemperatureSetpointHoldDuration::Id);
    optionalAttributes.ThermostatRunningState = emberAfContainsAttribute(endpointId, Thermostat::Id, ThermostatRunningState::Id);
    optionalAttributes.SetpointChangeSource   = emberAfContainsAttribute(endpointId, Thermostat::Id, SetpointChangeSource::Id);
    optionalAttributes.SetpointChangeAmount   = emberAfContainsAttribute(endpointId, Thermostat::Id, SetpointChangeAmount::Id);
    optionalAttributes.SetpointChangeSourceTimestamp =
        emberAfContainsAttribute(endpointId, Thermostat::Id, SetpointChangeSourceTimestamp::Id);
    optionalAttributes.SetpointHoldExpiryTimestamp =
        emberAfContainsAttribute(endpointId, Thermostat::Id, SetpointHoldExpiryTimestamp::Id);
    optionalAttributes.OutdoorTemperature = emberAfContainsAttribute(endpointId, Thermostat::Id, OutdoorTemperature::Id);
    return optionalAttributes;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

void __attribute__((weak)) MatterThermostatClusterInitCallback(EndpointId endpointId) {}

void __attribute__((weak)) MatterThermostatPluginServerInitCallback() {}

void __attribute__((weak))
MatterThermostatClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType clusterShutdownType)
{}
