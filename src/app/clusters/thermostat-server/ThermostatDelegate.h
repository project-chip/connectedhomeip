/**
 *    Copyright (c) 2024-2025 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <protocols/interaction_model/StatusCode.h>

#include "Setpoints.h"

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

/**
 * @brief Core Thermostat Delegate for basic thermostat functionality:
 *        setpoints, temperature, system mode, running mode, running state, etc.
 */
class Delegate
{
public:
    Delegate()          = default;
    virtual ~Delegate() = default;

    virtual SystemModeEnum GetSystemMode() const                                                          = 0;
    virtual bool SetSystemMode(SystemModeEnum systemMode)                                                 = 0;
    virtual ControlSequenceOfOperationEnum GetControlSequenceOfOperation() const                          = 0;
    virtual bool SetControlSequenceOfOperation(ControlSequenceOfOperationEnum controlSequenceOfOperation) = 0;
    virtual ThermostatRunningModeEnum GetRunningMode() const                                              = 0;
    virtual bool SetRunningMode(ThermostatRunningModeEnum runningMode)                                    = 0;
    virtual BitMask<RelayStateBitmap> GetRunningState() const                                             = 0;
    virtual bool SetRunningState(BitMask<RelayStateBitmap> runningState)                                  = 0;
    virtual DataModel::Nullable<temperature> GetLocalTemperature() const                                  = 0;
    virtual bool SetLocalTemperature(DataModel::Nullable<temperature> localTemperature)                   = 0;
    virtual int8_t GetLocalTemperatureCalibration() const                                                 = 0;
    virtual bool SetLocalTemperatureCalibration(int8_t localTemperatureCalibration)                       = 0;

    virtual BitMask<RemoteSensingBitmap> GetRemoteSensing() const             = 0;
    virtual bool SetRemoteSensing(BitMask<RemoteSensingBitmap> remoteSensing) = 0;

    virtual TemperatureSetpointHoldEnum GetTemperatureSetpointHold() const                                         = 0;
    virtual bool SetTemperatureSetpointHold(TemperatureSetpointHoldEnum temperatureSetpointHold)                   = 0;
    virtual DataModel::Nullable<uint16_t> GetTemperatureSetpointHoldDuration() const                               = 0;
    virtual bool SetTemperatureSetpointHoldDuration(DataModel::Nullable<uint16_t> temperatureSetpointHoldDuration) = 0;
    virtual DataModel::Nullable<uint32_t> GetSetpointHoldExpiryTimestamp() const                                   = 0;
    virtual bool SetSetpointHoldExpiryTimestamp(DataModel::Nullable<uint32_t> setpointHoldExpiryTimestamp)         = 0;

    virtual Protocols::InteractionModel::Status LoadSetpoints(Setpoints & setpoints)                                     = 0;
    virtual Protocols::InteractionModel::Status SaveSetpoint(const Setpoint & oldSetpoint, const Setpoint & newSetpoint) = 0;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
