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
#include "Temperature.h"

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

    virtual DataModel::Nullable<temperature> GetLocalTemperature() const            = 0;
    virtual Protocols::InteractionModel::Status SetLocalTemperature(DataModel::Nullable<temperature> localTemperature,
                                                                    bool & changed) = 0;

    virtual Protocols::InteractionModel::Status GetOutdoorTemperature(DataModel::Nullable<temperature> & outdoorTemp) const;

    virtual SystemModeEnum GetSystemMode() const                                                         = 0;
    virtual Protocols::InteractionModel::Status SetSystemMode(SystemModeEnum systemMode, bool & changed) = 0;

    virtual ControlSequenceOfOperationEnum GetControlSequenceOfOperation() const = 0;
    virtual Protocols::InteractionModel::Status
    SetControlSequenceOfOperation(ControlSequenceOfOperationEnum controlSequenceOfOperation, bool & changed) = 0;

    virtual Protocols::InteractionModel::Status GetRunningMode(ThermostatRunningModeEnum & runningMode) const = 0;
    virtual Protocols::InteractionModel::Status SetRunningMode(ThermostatRunningModeEnum runningMode, bool & changed) = 0;

    virtual Protocols::InteractionModel::Status GetRunningState(BitMask<RelayStateBitmap> & runningState) const = 0;
    virtual Protocols::InteractionModel::Status SetRunningState(BitMask<RelayStateBitmap> runningState, bool & changed) = 0;

    virtual int8_t GetLocalTemperatureCalibration() const { return 0; };
    virtual Protocols::InteractionModel::Status SetLocalTemperatureCalibration(int8_t localTemperatureCalibration, bool & changed)
    {
        changed = false;
        return Protocols::InteractionModel::Status::Success;
    };

    virtual Protocols::InteractionModel::Status GetRemoteSensing(BitMask<RemoteSensingBitmap> & remoteSensing) const;
    virtual Protocols::InteractionModel::Status SetRemoteSensing(BitMask<RemoteSensingBitmap> remoteSensing, bool & changed) = 0;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
