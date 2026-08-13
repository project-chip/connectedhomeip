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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/attribute-storage.h>
#include <app/util/attribute-table.h>
#include <app/util/endpoint-config-api.h>

#include "Temperature.h"

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::Protocols::InteractionModel;

ThermostatCluster::OptionalAttributes BaseIntegrationDelegate::GetOptionalAttributes(EndpointId endpointId,
                                                                                     BitFlags<Thermostat::Feature> features)
{
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
    return optionalAttributes;
}

ThermostatCluster::DefaultValues BaseIntegrationDelegate::LoadDefaultValues(EndpointId endpointId,
                                                                            const BitFlags<Thermostat::Feature> & features)
{
    ThermostatCluster::DefaultValues defaultValues;
    if (features.Has(Thermostat::Feature::kHeating))
    {
        if (auto status = AbsMinHeatSetpointLimit::GetDefault(endpointId, &defaultValues.absMinHeatSetpointLimit);
            status != Status::Success)
        {
            defaultValues.absMinHeatSetpointLimit = kDefaultAbsMinHeatSetpointLimit;
        }

        if (auto status = AbsMaxHeatSetpointLimit::GetDefault(endpointId, &defaultValues.absMaxHeatSetpointLimit);
            status != Status::Success)
        {
            defaultValues.absMaxHeatSetpointLimit = kDefaultAbsMaxHeatSetpointLimit;
        }

        temperature minHeatSetpointLimit;
        if (auto status = MinHeatSetpointLimit::GetDefault(endpointId, &minHeatSetpointLimit); status == Status::Success)
        {
            defaultValues.minHeatSetpointLimit.SetValue(minHeatSetpointLimit);
        }

        temperature maxHeatSetpointLimit;
        if (auto status = MaxHeatSetpointLimit::GetDefault(endpointId, &maxHeatSetpointLimit); status == Status::Success)
        {
            defaultValues.maxHeatSetpointLimit.SetValue(maxHeatSetpointLimit);
        }

        if (auto status = OccupiedHeatingSetpoint::GetDefault(endpointId, &defaultValues.occupiedHeatingSetpoint);
            status != Status::Success)
        {
            defaultValues.occupiedHeatingSetpoint = kDefaultHeatingSetpoint;
        }

        if (features.Has(Thermostat::Feature::kOccupancy))
        {
            temperature unoccupiedHeatingSetpoint;
            if (auto status = UnoccupiedHeatingSetpoint::GetDefault(endpointId, &unoccupiedHeatingSetpoint);
                status != Status::Success)
            {
                defaultValues.unoccupiedHeatingSetpoint = kDefaultHeatingSetpoint;
            }
        }
    }

    if (features.Has(Thermostat::Feature::kCooling))
    {
        if (auto status = AbsMinCoolSetpointLimit::GetDefault(endpointId, &defaultValues.absMinCoolSetpointLimit);
            status != Status::Success)
        {
            defaultValues.absMinCoolSetpointLimit = kDefaultAbsMinCoolSetpointLimit;
        }

        if (auto status = AbsMaxCoolSetpointLimit::GetDefault(endpointId, &defaultValues.absMaxCoolSetpointLimit);
            status != Status::Success)
        {
            defaultValues.absMaxCoolSetpointLimit = kDefaultAbsMaxCoolSetpointLimit;
        }

        temperature minCoolSetpointLimit;
        if (auto status = MinCoolSetpointLimit::GetDefault(endpointId, &minCoolSetpointLimit); status == Status::Success)
        {
            defaultValues.minCoolSetpointLimit.SetValue(minCoolSetpointLimit);
        }

        temperature maxCoolSetpointLimit;
        if (auto status = MaxCoolSetpointLimit::GetDefault(endpointId, &maxCoolSetpointLimit); status == Status::Success)
        {
            defaultValues.maxCoolSetpointLimit.SetValue(maxCoolSetpointLimit);
        }

        if (auto status = OccupiedCoolingSetpoint::GetDefault(endpointId, &defaultValues.occupiedCoolingSetpoint);
            status != Status::Success)
        {
            defaultValues.occupiedCoolingSetpoint = kDefaultCoolingSetpoint;
        }

        if (features.Has(Thermostat::Feature::kOccupancy))
        {
            temperature unoccupiedCoolingSetpoint;
            if (auto status = UnoccupiedCoolingSetpoint::GetDefault(endpointId, &unoccupiedCoolingSetpoint);
                status != Status::Success)
            {
                defaultValues.unoccupiedCoolingSetpoint = kDefaultCoolingSetpoint;
            }
        }
    }

    if (auto status = LocalTemperatureCalibration::GetDefault(endpointId, &defaultValues.localTemperatureCalibration);
        status != Status::Success)
    {
        defaultValues.localTemperatureCalibration = kDefaultLocalTemperatureCalibration;
    }

    auto hasHeating = features.Has(Feature::kHeating);
    auto hasCooling = features.Has(Feature::kCooling);

    if (auto status = SystemMode::GetDefault(endpointId, &defaultValues.systemMode); status != Status::Success)
    {
        if (hasHeating && hasCooling)
        {
            defaultValues.systemMode = SystemModeEnum::kAuto;
        }
        else if (hasHeating)
        {
            defaultValues.systemMode = SystemModeEnum::kHeat;
        }
        else if (hasCooling)
        {
            defaultValues.systemMode = SystemModeEnum::kCool;
        }
    }

    if (auto status = ControlSequenceOfOperation::GetDefault(endpointId, &defaultValues.controlSequenceOfOperation);
        status != Status::Success)
    {
        if (hasHeating && hasCooling)
        {
            defaultValues.controlSequenceOfOperation = ControlSequenceOfOperationEnum::kCoolingAndHeating;
        }
        else if (hasHeating)
        {
            defaultValues.controlSequenceOfOperation = ControlSequenceOfOperationEnum::kHeatingOnly;
        }
        else if (hasCooling)
        {
            defaultValues.controlSequenceOfOperation = ControlSequenceOfOperationEnum::kCoolingOnly;
        }
    }

    if (emberAfContainsAttribute(endpointId, Thermostat::Id, TemperatureSetpointHold::Id))
    {
        if (auto status = TemperatureSetpointHold::GetDefault(endpointId, &defaultValues.temperatureSetpointHold);
            status != Status::Success)
        {
            defaultValues.temperatureSetpointHold = TemperatureSetpointHoldEnum::kSetpointHoldOff;
        }
    }

    if (emberAfContainsAttribute(endpointId, Thermostat::Id, TemperatureSetpointHoldDuration::Id))
    {
        if (auto status =
                TemperatureSetpointHoldDuration::GetDefault(endpointId, defaultValues.temperatureSetpointHoldDuration);
            status != Status::Success)
        {
            defaultValues.temperatureSetpointHoldDuration = 0;
        }
    }

    if (emberAfContainsAttribute(endpointId, Thermostat::Id, SetpointHoldExpiryTimestamp::Id))
    {
        if (auto status = SetpointHoldExpiryTimestamp::GetDefault(endpointId, defaultValues.setpointHoldExpiryTimestamp);
            status != Status::Success)
        {
            defaultValues.setpointHoldExpiryTimestamp.SetNull();
        }
    }

    return defaultValues;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

void __attribute__((weak)) MatterThermostatClusterInitCallback(chip::EndpointId endpointId) {}

void __attribute__((weak)) MatterThermostatPluginServerInitCallback() {}

void __attribute__((weak))
MatterThermostatClusterShutdownCallback(chip::EndpointId endpointId, MatterClusterShutdownType clusterShutdownType)
{}
