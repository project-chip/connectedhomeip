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

#include "CodegenIntegration.h"

#if MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT > 0

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/thermostat-server/CodegenIntegration.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {
namespace Chef {

namespace {

using HeatingCoolingOccupancyThermostatCluster =
    ThermostatCluster<Thermostat::Delegate, ThermostatHeatingSetpoints::Delegate, ThermostatCoolingSetpoints::Delegate,
                      ThermostatAutoSetpoints::Delegate, ThermostatHold::Delegate, ThermostatOccupancy::Delegate>;

} // namespace

void LoadDefaultsFromZAP(EndpointId endpointId, ChefThermostatDelegate & delegate)
{
    bool changed = false;

    DataModel::Nullable<temperature> localTemperature = DataModel::NullNullable;
    if (Attributes::LocalTemperature::GetDefault(endpointId, localTemperature) == Status::Success)
    {
        delegate.SetLocalTemperature(localTemperature, changed);
    }

    DataModel::Nullable<temperature> outdoorTemperature = DataModel::NullNullable;
    if (Attributes::OutdoorTemperature::GetDefault(endpointId, outdoorTemperature) == Status::Success)
    {
        delegate.SetOutdoorTemperatureValue(outdoorTemperature, changed);
    }

    int8_t localTemperatureCalibration = kDefaultLocalTemperatureCalibration;
    if (Attributes::LocalTemperatureCalibration::GetDefault(endpointId, localTemperatureCalibration) == Status::Success)
    {
        delegate.SetLocalTemperatureCalibration(localTemperatureCalibration, changed);
    }

    BitMask<RemoteSensingBitmap> remoteSensing(0);
    if (Attributes::RemoteSensing::GetDefault(endpointId, remoteSensing) == Status::Success)
    {
        delegate.SetRemoteSensing(remoteSensing, changed);
    }

    ControlSequenceOfOperationEnum controlSequenceOfOperation = ControlSequenceOfOperationEnum::kCoolingAndHeating;
    if (Attributes::ControlSequenceOfOperation::GetDefault(endpointId, controlSequenceOfOperation) == Status::Success)
    {
        delegate.SetControlSequenceOfOperation(controlSequenceOfOperation, changed);
    }

    SystemModeEnum systemMode = SystemModeEnum::kAuto;
    if (Attributes::SystemMode::GetDefault(endpointId, systemMode) == Status::Success)
    {
        delegate.SetSystemMode(systemMode, changed);
    }

    ThermostatRunningModeEnum runningMode = ThermostatRunningModeEnum::kOff;
    if (Attributes::ThermostatRunningMode::GetDefault(endpointId, runningMode) == Status::Success)
    {
        delegate.SetRunningMode(runningMode, changed);
    }

    BitMask<RelayStateBitmap> runningState(0);
    if (Attributes::ThermostatRunningState::GetDefault(endpointId, runningState) == Status::Success)
    {
        delegate.SetRunningState(runningState, changed);
    }
}

void LoadDefaultsFromZAP(EndpointId endpointId, ChefThermostatSetpointsDelegate & delegate)
{
    bool changed = false;

    temperature absMinHeatSetpointLimit = kDefaultAbsMinHeatSetpointLimit;
    Attributes::AbsMinHeatSetpointLimit::GetDefault(endpointId, absMinHeatSetpointLimit);
    delegate.SetAbsMinHeatSetpointLimit(absMinHeatSetpointLimit);

    temperature absMaxHeatSetpointLimit = kDefaultAbsMaxHeatSetpointLimit;
    Attributes::AbsMaxHeatSetpointLimit::GetDefault(endpointId, absMaxHeatSetpointLimit);
    delegate.SetAbsMaxHeatSetpointLimit(absMaxHeatSetpointLimit);

    temperature minHeatSetpointLimit = absMinHeatSetpointLimit;
    Attributes::MinHeatSetpointLimit::GetDefault(endpointId, minHeatSetpointLimit);
    delegate.SetMinHeatSetpointLimit(minHeatSetpointLimit, changed);

    temperature maxHeatSetpointLimit = absMaxHeatSetpointLimit;
    Attributes::MaxHeatSetpointLimit::GetDefault(endpointId, maxHeatSetpointLimit);
    delegate.SetMaxHeatSetpointLimit(maxHeatSetpointLimit, changed);

    temperature occupiedHeatingSetpoint = kDefaultHeatingSetpoint;
    Attributes::OccupiedHeatingSetpoint::GetDefault(endpointId, occupiedHeatingSetpoint);
    delegate.SetOccupiedHeatingSetpoint(occupiedHeatingSetpoint, changed);

    temperature unoccupiedHeatingSetpoint = kDefaultHeatingSetpoint;
    Attributes::UnoccupiedHeatingSetpoint::GetDefault(endpointId, unoccupiedHeatingSetpoint);
    delegate.SetUnoccupiedHeatingSetpoint(unoccupiedHeatingSetpoint, changed);

    temperature absMinCoolSetpointLimit = kDefaultAbsMinCoolSetpointLimit;
    Attributes::AbsMinCoolSetpointLimit::GetDefault(endpointId, absMinCoolSetpointLimit);
    delegate.SetAbsMinCoolSetpointLimit(absMinCoolSetpointLimit);

    temperature absMaxCoolSetpointLimit = kDefaultAbsMaxCoolSetpointLimit;
    Attributes::AbsMaxCoolSetpointLimit::GetDefault(endpointId, absMaxCoolSetpointLimit);
    delegate.SetAbsMaxCoolSetpointLimit(absMaxCoolSetpointLimit);

    temperature minCoolSetpointLimit = absMinCoolSetpointLimit;
    Attributes::MinCoolSetpointLimit::GetDefault(endpointId, minCoolSetpointLimit);
    delegate.SetMinCoolSetpointLimit(minCoolSetpointLimit, changed);

    temperature maxCoolSetpointLimit = absMaxCoolSetpointLimit;
    Attributes::MaxCoolSetpointLimit::GetDefault(endpointId, maxCoolSetpointLimit);
    delegate.SetMaxCoolSetpointLimit(maxCoolSetpointLimit, changed);

    temperature occupiedCoolingSetpoint = kDefaultCoolingSetpoint;
    Attributes::OccupiedCoolingSetpoint::GetDefault(endpointId, occupiedCoolingSetpoint);
    delegate.SetOccupiedCoolingSetpoint(occupiedCoolingSetpoint, changed);

    temperature unoccupiedCoolingSetpoint = kDefaultCoolingSetpoint;
    Attributes::UnoccupiedCoolingSetpoint::GetDefault(endpointId, unoccupiedCoolingSetpoint);
    delegate.SetUnoccupiedCoolingSetpoint(unoccupiedCoolingSetpoint, changed);

    int8_t deadBand = static_cast<int8_t>(kDefaultDeadBand / 10);
    if (Attributes::MinSetpointDeadBand::GetDefault(endpointId, deadBand) == Status::Success)
    {
        delegate.SetMinDeadband(static_cast<temperature>(deadBand) * 10, changed);
    }
}

void LoadDefaultsFromZAP(EndpointId endpointId, ChefThermostatHoldDelegate & delegate)
{
    bool changed = false;

    TemperatureSetpointHoldEnum temperatureSetpointHold = TemperatureSetpointHoldEnum::kSetpointHoldOff;
    if (Attributes::TemperatureSetpointHold::GetDefault(endpointId, temperatureSetpointHold) == Status::Success)
    {
        delegate.SetTemperatureSetpointHold(temperatureSetpointHold, changed);
    }

    DataModel::Nullable<uint16_t> duration = DataModel::NullNullable;
    if (Attributes::TemperatureSetpointHoldDuration::GetDefault(endpointId, duration) == Status::Success)
    {
        delegate.SetTemperatureSetpointHoldDuration(duration, changed);
    }

    DataModel::Nullable<uint32_t> expiryTimestamp = DataModel::NullNullable;
    if (Attributes::SetpointHoldExpiryTimestamp::GetDefault(endpointId, expiryTimestamp) == Status::Success)
    {
        delegate.SetSetpointHoldExpiryTimestamp(expiryTimestamp, changed);
    }
}

void LoadDefaultsFromZAP(EndpointId endpointId, ChefThermostatOccupancyDelegate & delegate)
{
    bool changed = false;

    BitMask<OccupancyBitmap> occupancy(OccupancyBitmap::kOccupied);
    if (Attributes::Occupancy::GetDefault(endpointId, occupancy) == Status::Success)
    {
        delegate.SetOccupancy(occupancy, changed);
    }
}

BitFlags<Feature> LoadFeaturesFromZAP(EndpointId endpointId)
{
    uint32_t rawFeatureMap = 0;
    if (Attributes::FeatureMap::GetDefault(endpointId, rawFeatureMap) != Status::Success)
    {
        rawFeatureMap = to_underlying(Feature::kHeating);
    }
    return BitFlags<Feature>(rawFeatureMap);
}

CHIP_ERROR InitThermostatClusterOnEndpoint(EndpointId endpointId)
{
    VerifyOrReturnValue(FindClusterOnEndpoint(endpointId) == nullptr, CHIP_NO_ERROR);

    auto * thermostatDelegate = Platform::New<ChefThermostatDelegate>(endpointId);
    auto * setpointsDelegate  = Platform::New<ChefThermostatSetpointsDelegate>(endpointId);
    auto * holdDelegate       = Platform::New<ChefThermostatHoldDelegate>();
    auto * occupancyDelegate  = Platform::New<ChefThermostatOccupancyDelegate>();
    VerifyOrReturnError(thermostatDelegate != nullptr && setpointsDelegate != nullptr && holdDelegate != nullptr &&
                            occupancyDelegate != nullptr,
                        CHIP_ERROR_NO_MEMORY);

    LoadDefaultsFromZAP(endpointId, *thermostatDelegate);
    LoadDefaultsFromZAP(endpointId, *setpointsDelegate);
    LoadDefaultsFromZAP(endpointId, *holdDelegate);
    LoadDefaultsFromZAP(endpointId, *occupancyDelegate);

    BitFlags<Feature> features = LoadFeaturesFromZAP(endpointId);

    Thermostat::Delegate & baseDelegate                    = *thermostatDelegate;
    ThermostatHeatingSetpoints::Delegate & heatingDelegate = *setpointsDelegate;
    ThermostatCoolingSetpoints::Delegate & coolingDelegate = *setpointsDelegate;
    ThermostatAutoSetpoints::Delegate & autoDelegate       = *setpointsDelegate;
    ThermostatHold::Delegate & holdDel                     = *holdDelegate;
    ThermostatOccupancy::Delegate & occDel                 = *occupancyDelegate;

    constexpr BitFlags<Feature> kHeatingOnly(Feature::kHeating);
    constexpr BitFlags<Feature> kHeatingCooling(Feature::kHeating, Feature::kCooling);
    constexpr BitFlags<Feature> kHeatingCoolingAuto(Feature::kHeating, Feature::kCooling, Feature::kAutoMode);
    constexpr BitFlags<Feature> kHeatingCoolingAutoOccupancy(Feature::kHeating, Feature::kCooling, Feature::kAutoMode,
                                                             Feature::kOccupancy);

    if (features == kHeatingOnly)
    {
        Thermostat::ServerInit(endpointId, baseDelegate, heatingDelegate, holdDel);
    }
    else if (features == kHeatingCooling)
    {
        Thermostat::ServerInit(endpointId, baseDelegate, heatingDelegate, coolingDelegate, holdDel);
    }
    else if (features == kHeatingCoolingAuto)
    {
        Thermostat::ServerInit(endpointId, baseDelegate, heatingDelegate, coolingDelegate, autoDelegate, holdDel);
    }
    else if (features == kHeatingCoolingAutoOccupancy)
    {
        Thermostat::ServerInit(endpointId, baseDelegate, heatingDelegate, coolingDelegate, autoDelegate, holdDel, occDel);
    }
    else
    {
        VerifyOrDieWithMsg(false, Zcl, "Unsupported or invalid Thermostat FeatureMap: 0x%08" PRIx32 " on endpoint %u",
                           features.Raw(), endpointId);
    }

    VerifyOrReturnError(FindClusterOnEndpoint(endpointId) != nullptr, CHIP_ERROR_INTERNAL);
    return CHIP_NO_ERROR;
}

ThermostatClusterBase * FindClusterOnEndpoint(EndpointId endpointId)
{
    ServerClusterInterface * cluster =
        CodegenDataModelProvider::Instance().Registry().Get(ConcreteClusterPath(endpointId, Thermostat::Id));
    return static_cast<ThermostatClusterBase *>(cluster);
}

Status SetOccupancy(EndpointId endpointId, BitMask<OccupancyBitmap> occupancy)
{
    auto * cluster = Thermostat::FindClusterOnEndpoint<HeatingCoolingOccupancyThermostatCluster>(endpointId);
    VerifyOrReturnValue(cluster != nullptr, Status::UnsupportedAttribute);
    return cluster->SetOccupancy(occupancy);
}

} // namespace Chef
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

#endif // MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT > 0
