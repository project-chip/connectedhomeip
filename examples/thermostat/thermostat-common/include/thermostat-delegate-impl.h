/*
 *    Copyright (c) 2024-2026 Project CHIP Authors
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

#include <app/persistence/AttributePersistenceProvider.h>
#include <app/persistence/AttributePersistenceProviderInstance.h>

#include <app/clusters/thermostat-server/PresetStructWithOwnedMembers.h>
#include <app/clusters/thermostat-server/ThermostatClusterOccupancy.h>
#include <app/clusters/thermostat-server/ThermostatClusterPresets.h>
#include <app/clusters/thermostat-server/ThermostatClusterSuggestions.h>
#include <app/clusters/thermostat-server/ThermostatClusterWithFeatures.h>
#include <app/clusters/thermostat-server/ThermostatDelegate.h>
#include <app/clusters/thermostat-server/ThermostatSuggestionStructWithOwnedMembers.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class ThermostatDelegate : public Delegate
{
public:
    ThermostatDelegate(EndpointId endpoint, AttributePersistenceProvider * provider = nullptr)
        : mEndpointId(endpoint),
          mProvider(provider) {}

    SystemModeEnum GetSystemMode() const override;
    bool SetSystemMode(SystemModeEnum systemMode) override;
    ThermostatRunningModeEnum GetRunningMode() const override;
    bool SetRunningMode(ThermostatRunningModeEnum runningMode) override;
    BitMask<RelayStateBitmap> GetRunningState() const override;
    bool SetRunningState(BitMask<RelayStateBitmap> runningState) override;
    ControlSequenceOfOperationEnum GetControlSequenceOfOperation() const override;
    bool SetControlSequenceOfOperation(ControlSequenceOfOperationEnum controlSequenceOfOperation) override;
    DataModel::Nullable<temperature> GetLocalTemperature() const override;
    bool SetLocalTemperature(DataModel::Nullable<temperature> localTemperature) override;
    int8_t GetLocalTemperatureCalibration() const override;
    bool SetLocalTemperatureCalibration(int8_t localTemperatureCalibration) override;

    BitMask<RemoteSensingBitmap> GetRemoteSensing() const override;

    bool SetRemoteSensing(BitMask<RemoteSensingBitmap> remoteSensing) override;
    TemperatureSetpointHoldEnum GetTemperatureSetpointHold() const override;
    bool SetTemperatureSetpointHold(TemperatureSetpointHoldEnum temperatureSetpointHold) override;
    DataModel::Nullable<uint16_t> GetTemperatureSetpointHoldDuration() const override;
    bool SetTemperatureSetpointHoldDuration(DataModel::Nullable<uint16_t> temperatureSetpointHoldDuration) override;
    DataModel::Nullable<uint32_t> GetSetpointHoldExpiryTimestamp() const override;
    bool SetSetpointHoldExpiryTimestamp(DataModel::Nullable<uint32_t> setpointHoldExpiryTimestamp) override;

    Protocols::InteractionModel::Status LoadSetpoints(Setpoints & setpoints) override;
    Protocols::InteractionModel::Status SaveSetpoint(const Setpoint & oldSetpoint, const Setpoint & newSetpoint) override;

private:
    EndpointId mEndpointId;
    AttributePersistenceProvider * mProvider = nullptr;

    ControlSequenceOfOperationEnum mControlSequenceOfOperation = ControlSequenceOfOperationEnum::kCoolingAndHeating;

    SystemModeEnum mSystemMode                         = SystemModeEnum::kOff;
    ThermostatRunningModeEnum mRunningMode             = ThermostatRunningModeEnum::kOff;
    BitMask<RelayStateBitmap> mRunningState            = BitMask<RelayStateBitmap>(0);
    DataModel::Nullable<temperature> mLocalTemperature = DataModel::Nullable<int16_t>();
    int8_t mLocalTemperatureCalibration                = 0;

    BitMask<RemoteSensingBitmap> mRemoteSensing = BitMask<RemoteSensingBitmap>(0);

    TemperatureSetpointHoldEnum mTemperatureSetpointHold = TemperatureSetpointHoldEnum::kSetpointHoldOff;
    DataModel::Nullable<uint16_t> mTemperatureSetpointHoldDuration;
    DataModel::Nullable<uint32_t> mSetpointHoldExpiryTimestamp;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
