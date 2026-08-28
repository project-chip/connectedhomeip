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

#include "app/clusters/thermostat-server/Temperature.h"
#include "lib/core/CHIPError.h"
#include <app/persistence/AttributePersistenceProvider.h>
#include <app/persistence/AttributePersistenceProviderInstance.h>

#include <app/clusters/thermostat-server/ThermostatCluster.h>
#include <app/clusters/thermostat-server/ThermostatDelegate.h>

namespace chip {
class FabricTable;

namespace app {
namespace Clusters {
namespace Thermostat {

class ThermostatDelegate : public Delegate
{
public:
    ThermostatDelegate(EndpointId endpoint, AttributePersistenceProvider * provider = nullptr,
                       FabricTable * fabricTable = nullptr) :
        mEndpointId(endpoint),
        mProvider(provider), mFabricTable(fabricTable)
    {}

    FabricTable * GetFabricTable() const override;

    CHIP_ERROR Startup(ServerClusterContext & context) override;

    SystemModeEnum GetSystemMode() const override;
    Protocols::InteractionModel::Status SetSystemMode(SystemModeEnum systemMode, bool & changed) override;

    Protocols::InteractionModel::Status GetRunningMode(ThermostatRunningModeEnum & runningMode) const override;
    Protocols::InteractionModel::Status SetRunningMode(ThermostatRunningModeEnum runningMode, bool & changed) override;

    Protocols::InteractionModel::Status GetRunningState(BitMask<RelayStateBitmap> & runningState) const override;
    Protocols::InteractionModel::Status SetRunningState(BitMask<RelayStateBitmap> runningState, bool & changed) override;

    ControlSequenceOfOperationEnum GetControlSequenceOfOperation() const override;
    Protocols::InteractionModel::Status SetControlSequenceOfOperation(ControlSequenceOfOperationEnum seq, bool & changed) override;

    DataModel::Nullable<temperature> GetLocalTemperature() const override;
    Protocols::InteractionModel::Status SetLocalTemperature(DataModel::Nullable<temperature> temp, bool & changed) override;

    Protocols::InteractionModel::Status GetOutdoorTemperature(DataModel::Nullable<temperature> & outdoorTemp) const override;

    int8_t GetLocalTemperatureCalibration() const override;
    Protocols::InteractionModel::Status SetLocalTemperatureCalibration(int8_t temp, bool & changed) override;

    Protocols::InteractionModel::Status GetRemoteSensing(BitMask<RemoteSensingBitmap> & remoteSensing) const override;
    Protocols::InteractionModel::Status SetRemoteSensing(BitMask<RemoteSensingBitmap> sensing, bool & changed) override;

private:
    EndpointId mEndpointId;
    AttributePersistenceProvider * mProvider = nullptr;
    FabricTable * mFabricTable               = nullptr;

    ControlSequenceOfOperationEnum mControlSequenceOfOperation = ControlSequenceOfOperationEnum::kCoolingAndHeating;

    SystemModeEnum mSystemMode                         = SystemModeEnum::kOff;
    ThermostatRunningModeEnum mRunningMode             = ThermostatRunningModeEnum::kOff;
    BitMask<RelayStateBitmap> mRunningState            = BitMask<RelayStateBitmap>(0);
    DataModel::Nullable<temperature> mLocalTemperature = DataModel::Nullable<int16_t>();
    int8_t mLocalTemperatureCalibration                = 0;

    BitMask<RemoteSensingBitmap> mRemoteSensing = BitMask<RemoteSensingBitmap>(0);
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
