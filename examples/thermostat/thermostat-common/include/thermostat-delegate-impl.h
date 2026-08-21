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
    ThermostatDelegate(EndpointId endpoint, AttributePersistenceProvider * provider = nullptr) :
        mEndpointId(endpoint), mProvider(provider)
    {}

    SystemModeEnum GetSystemMode() const override;
    Protocols::InteractionModel::Status SetSystemMode(SystemModeEnum systemMode, bool & changed) override;

    ThermostatRunningModeEnum GetRunningMode() const override;
    Protocols::InteractionModel::Status SetRunningMode(ThermostatRunningModeEnum runningMode, bool & changed) override;

    BitMask<RelayStateBitmap> GetRunningState() const override;
    Protocols::InteractionModel::Status SetRunningState(BitMask<RelayStateBitmap> runningState, bool & changed) override;

    ControlSequenceOfOperationEnum GetControlSequenceOfOperation() const override;
    Protocols::InteractionModel::Status SetControlSequenceOfOperation(ControlSequenceOfOperationEnum seq, bool & changed) override;

    DataModel::Nullable<temperature> GetLocalTemperature() const override;
    Protocols::InteractionModel::Status SetLocalTemperature(DataModel::Nullable<temperature> temp, bool & changed) override;

    int8_t GetLocalTemperatureCalibration() const override;
    Protocols::InteractionModel::Status SetLocalTemperatureCalibration(int8_t temp, bool & changed) override;

    BitMask<RemoteSensingBitmap> GetRemoteSensing() const override;
    Protocols::InteractionModel::Status SetRemoteSensing(BitMask<RemoteSensingBitmap> sensing, bool & changed) override;

    TemperatureSetpointHoldEnum GetTemperatureSetpointHold() const override;
    Protocols::InteractionModel::Status SetTemperatureSetpointHold(TemperatureSetpointHoldEnum hold, bool & changed) override;

    DataModel::Nullable<uint16_t> GetTemperatureSetpointHoldDuration() const override;
    Protocols::InteractionModel::Status SetTemperatureSetpointHoldDuration(DataModel::Nullable<uint16_t> duration,
                                                                           bool & changed) override;

    DataModel::Nullable<uint32_t> GetSetpointHoldExpiryTimestamp() const override;
    Protocols::InteractionModel::Status SetSetpointHoldExpiryTimestamp(DataModel::Nullable<uint32_t> timestamp,
                                                                       bool & changed) override;

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
