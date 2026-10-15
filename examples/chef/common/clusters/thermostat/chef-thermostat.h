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

#pragma once

#include <app/util/config.h>

#if MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT > 0

#include <app/clusters/thermostat-server/ThermostatCluster.h>
#include <app/data-model/Nullable.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitMask.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {
namespace Chef {

class ChefThermostatDelegate : public Thermostat::Delegate
{
public:
    explicit ChefThermostatDelegate(EndpointId endpointId) : mEndpointId(endpointId) {}
    ~ChefThermostatDelegate() override = default;

    FabricTable & GetFabricTable() const override;

    DataModel::Nullable<temperature> GetLocalTemperature() const override;
    Protocols::InteractionModel::Status SetLocalTemperature(DataModel::Nullable<temperature> localTemperature,
                                                            bool & changed) override;

    Protocols::InteractionModel::Status GetOutdoorTemperature(DataModel::Nullable<temperature> & outdoorTemperature) const override;
    void SetOutdoorTemperatureValue(DataModel::Nullable<temperature> outdoorTemperature, bool & changed);

    int8_t GetLocalTemperatureCalibration() const override;
    Protocols::InteractionModel::Status SetLocalTemperatureCalibration(int8_t localTemperatureCalibration, bool & changed) override;

    Protocols::InteractionModel::Status GetRemoteSensing(BitMask<RemoteSensingBitmap> & remoteSensing) const override;
    Protocols::InteractionModel::Status SetRemoteSensing(BitMask<RemoteSensingBitmap> remoteSensing, bool & changed) override;

    ControlSequenceOfOperationEnum GetControlSequenceOfOperation() const override;
    Protocols::InteractionModel::Status SetControlSequenceOfOperation(ControlSequenceOfOperationEnum controlSequenceOfOperation,
                                                                      bool & changed) override;

    SystemModeEnum GetSystemMode() const override;
    Protocols::InteractionModel::Status SetSystemMode(SystemModeEnum systemMode, bool & changed) override;

    Protocols::InteractionModel::Status GetRunningMode(ThermostatRunningModeEnum & runningMode) const override;
    Protocols::InteractionModel::Status SetRunningMode(ThermostatRunningModeEnum runningMode, bool & changed) override;

    Protocols::InteractionModel::Status GetRunningState(BitMask<RelayStateBitmap> & runningState) const override;
    Protocols::InteractionModel::Status SetRunningState(BitMask<RelayStateBitmap> runningState, bool & changed) override;

private:
    [[maybe_unused]] EndpointId mEndpointId                    = kInvalidEndpointId;
    DataModel::Nullable<temperature> mLocalTemperature         = DataModel::NullNullable;
    DataModel::Nullable<temperature> mOutdoorTemperature       = DataModel::NullNullable;
    int8_t mLocalTemperatureCalibration                        = kDefaultLocalTemperatureCalibration;
    BitMask<RemoteSensingBitmap> mRemoteSensing                = BitMask<RemoteSensingBitmap>(0);
    ControlSequenceOfOperationEnum mControlSequenceOfOperation = ControlSequenceOfOperationEnum::kCoolingAndHeating;
    SystemModeEnum mSystemMode                                 = SystemModeEnum::kAuto;
    ThermostatRunningModeEnum mRunningMode                     = ThermostatRunningModeEnum::kOff;
    BitMask<RelayStateBitmap> mRunningState                    = BitMask<RelayStateBitmap>(0);
};

class ChefThermostatSetpointsDelegate : public ThermostatHeatingSetpoints::Delegate,
                                        public ThermostatCoolingSetpoints::Delegate,
                                        public ThermostatAutoSetpoints::Delegate
{
public:
    explicit ChefThermostatSetpointsDelegate(EndpointId endpointId) : mEndpointId(endpointId) {}
    ~ChefThermostatSetpointsDelegate() override = default;

    Protocols::InteractionModel::Status GetOccupiedHeatingSetpoint(temperature & occupiedHeatingSetpoint) const override;
    Protocols::InteractionModel::Status SetOccupiedHeatingSetpoint(temperature occupiedHeatingSetpoint, bool & changed) override;

    Protocols::InteractionModel::Status GetUnoccupiedHeatingSetpoint(temperature & unoccupiedHeatingSetpoint) const override;
    Protocols::InteractionModel::Status SetUnoccupiedHeatingSetpoint(temperature unoccupiedHeatingSetpoint,
                                                                     bool & changed) override;

    Protocols::InteractionModel::Status GetAbsMinHeatSetpointLimit(temperature & absMinHeatSetpointLimit) const override;
    Protocols::InteractionModel::Status GetAbsMaxHeatSetpointLimit(temperature & absMaxHeatSetpointLimit) const override;
    void SetAbsMinHeatSetpointLimit(temperature absMinHeatSetpointLimit) { mAbsMinHeatSetpointLimit = absMinHeatSetpointLimit; }
    void SetAbsMaxHeatSetpointLimit(temperature absMaxHeatSetpointLimit) { mAbsMaxHeatSetpointLimit = absMaxHeatSetpointLimit; }

    Protocols::InteractionModel::Status GetMinHeatSetpointLimit(temperature & minHeatSetpointLimit) const override;
    Protocols::InteractionModel::Status SetMinHeatSetpointLimit(temperature minHeatSetpointLimit, bool & changed) override;

    Protocols::InteractionModel::Status GetMaxHeatSetpointLimit(temperature & maxHeatSetpointLimit) const override;
    Protocols::InteractionModel::Status SetMaxHeatSetpointLimit(temperature maxHeatSetpointLimit, bool & changed) override;

    Protocols::InteractionModel::Status GetOccupiedCoolingSetpoint(temperature & occupiedCoolingSetpoint) const override;
    Protocols::InteractionModel::Status SetOccupiedCoolingSetpoint(temperature occupiedCoolingSetpoint, bool & changed) override;

    Protocols::InteractionModel::Status GetUnoccupiedCoolingSetpoint(temperature & unoccupiedCoolingSetpoint) const override;
    Protocols::InteractionModel::Status SetUnoccupiedCoolingSetpoint(temperature unoccupiedCoolingSetpoint,
                                                                     bool & changed) override;

    Protocols::InteractionModel::Status GetAbsMinCoolSetpointLimit(temperature & absMinCoolSetpointLimit) const override;
    Protocols::InteractionModel::Status GetAbsMaxCoolSetpointLimit(temperature & absMaxCoolSetpointLimit) const override;
    void SetAbsMinCoolSetpointLimit(temperature absMinCoolSetpointLimit) { mAbsMinCoolSetpointLimit = absMinCoolSetpointLimit; }
    void SetAbsMaxCoolSetpointLimit(temperature absMaxCoolSetpointLimit) { mAbsMaxCoolSetpointLimit = absMaxCoolSetpointLimit; }

    Protocols::InteractionModel::Status GetMinCoolSetpointLimit(temperature & minCoolSetpointLimit) const override;
    Protocols::InteractionModel::Status SetMinCoolSetpointLimit(temperature minCoolSetpointLimit, bool & changed) override;

    Protocols::InteractionModel::Status GetMaxCoolSetpointLimit(temperature & maxCoolSetpointLimit) const override;
    Protocols::InteractionModel::Status SetMaxCoolSetpointLimit(temperature maxCoolSetpointLimit, bool & changed) override;

    Protocols::InteractionModel::Status GetMinDeadband(temperature & minDeadband) const override;
    Protocols::InteractionModel::Status SetMinDeadband(temperature minDeadband, bool & changed);

private:
    [[maybe_unused]] EndpointId mEndpointId = kInvalidEndpointId;
    temperature mAbsMinHeatSetpointLimit    = kDefaultAbsMinHeatSetpointLimit;
    temperature mAbsMaxHeatSetpointLimit    = kDefaultAbsMaxHeatSetpointLimit;
    temperature mMinHeatSetpointLimit       = kDefaultAbsMinHeatSetpointLimit;
    temperature mMaxHeatSetpointLimit       = kDefaultAbsMaxHeatSetpointLimit;
    temperature mOccupiedHeatingSetpoint    = kDefaultHeatingSetpoint;
    temperature mUnoccupiedHeatingSetpoint  = kDefaultHeatingSetpoint;
    temperature mAbsMinCoolSetpointLimit    = kDefaultAbsMinCoolSetpointLimit;
    temperature mAbsMaxCoolSetpointLimit    = kDefaultAbsMaxCoolSetpointLimit;
    temperature mMinCoolSetpointLimit       = kDefaultAbsMinCoolSetpointLimit;
    temperature mMaxCoolSetpointLimit       = kDefaultAbsMaxCoolSetpointLimit;
    temperature mOccupiedCoolingSetpoint    = kDefaultCoolingSetpoint;
    temperature mUnoccupiedCoolingSetpoint  = kDefaultCoolingSetpoint;
    temperature mMinDeadband                = kDefaultDeadBand;
};

class ChefThermostatHoldDelegate : public ThermostatHold::Delegate
{
public:
    ChefThermostatHoldDelegate()           = default;
    ~ChefThermostatHoldDelegate() override = default;

    TemperatureSetpointHoldEnum GetTemperatureSetpointHold() const override;
    Protocols::InteractionModel::Status SetTemperatureSetpointHold(TemperatureSetpointHoldEnum temperatureSetpointHold,
                                                                   bool & changed) override;

    DataModel::Nullable<uint16_t> GetTemperatureSetpointHoldDuration() const override;
    Protocols::InteractionModel::Status SetTemperatureSetpointHoldDuration(DataModel::Nullable<uint16_t> duration,
                                                                           bool & changed) override;

    DataModel::Nullable<uint32_t> GetSetpointHoldExpiryTimestamp() const override;
    Protocols::InteractionModel::Status SetSetpointHoldExpiryTimestamp(DataModel::Nullable<uint32_t> timestamp,
                                                                       bool & changed) override;

private:
    TemperatureSetpointHoldEnum mTemperatureSetpointHold           = TemperatureSetpointHoldEnum::kSetpointHoldOff;
    DataModel::Nullable<uint16_t> mTemperatureSetpointHoldDuration = DataModel::NullNullable;
    DataModel::Nullable<uint32_t> mSetpointHoldExpiryTimestamp     = DataModel::NullNullable;
};

class ChefThermostatOccupancyDelegate : public ThermostatOccupancy::Delegate
{
public:
    ChefThermostatOccupancyDelegate()           = default;
    ~ChefThermostatOccupancyDelegate() override = default;

    BitMask<OccupancyBitmap> GetOccupancy() const override;
    Protocols::InteractionModel::Status SetOccupancy(BitMask<OccupancyBitmap> occupied, bool & changed) override;

private:
    BitMask<OccupancyBitmap> mOccupancy = BitMask<OccupancyBitmap>(OccupancyBitmap::kOccupied);
};

} // namespace Chef
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

#endif // MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT > 0
