/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "app/clusters/thermostat-server/thermostat-server.h"

#include "app/EventLogging.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;

void EmitSystemModeChangeEvent(EndpointId endpoint, Optional<SystemModeEnum> previousSystemMode, SystemModeEnum currentSystemMode)
{
    Events::SystemModeChange::Type event;
    EventNumber eventNumber;

    event.previousSystemMode = previousSystemMode;
    event.currentSystemMode  = currentSystemMode;

    CHIP_ERROR err = LogEvent(event, endpoint, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Failed to emit SystemModeChange event: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void EmitLocalTemperatureChangeEvent(EndpointId endpoint, DataModel::Nullable<int16_t> currentLocalTemperature)
{
    Events::LocalTemperatureChange::Type event;
    EventNumber eventNumber;

    event.currentLocalTemperature = currentLocalTemperature;

    CHIP_ERROR err = LogEvent(event, endpoint, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Failed to emit LocalTemperatureChange event: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void EmitOccupancyChangeEvent(EndpointId endpoint, Optional<chip::BitMask<OccupancyBitmap>> previousOccupancy,
                              chip::BitMask<OccupancyBitmap> currentOccupancy)
{
    Events::OccupancyChange::Type event;
    EventNumber eventNumber;

    event.previousOccupancy = previousOccupancy;
    event.currentOccupancy  = currentOccupancy;

    CHIP_ERROR err = LogEvent(event, endpoint, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Failed to emit OccupancyChange event: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void EmitSetpointChangeEvent(EndpointId endpoint, SystemModeEnum systemMode, Optional<chip::BitMask<OccupancyBitmap>> occupancy,
                             Optional<int16_t> previousSetpoint, int16_t currentSetpoint)
{
    Events::SetpointChange::Type event;
    EventNumber eventNumber;

    event.systemMode       = systemMode;
    event.occupancy        = occupancy;
    event.previousSetpoint = previousSetpoint;
    event.currentSetpoint  = currentSetpoint;

    CHIP_ERROR err = LogEvent(event, endpoint, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Failed to emit SetpointChange event: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void EmitRunningStateChangeEvent(EndpointId endpoint, Optional<chip::BitMask<RelayStateBitmap>> previousRunningState,
                                 chip::BitMask<RelayStateBitmap> currentRunningState)
{
    Events::RunningStateChange::Type event;
    EventNumber eventNumber;

    event.previousRunningState = previousRunningState;
    event.currentRunningState  = currentRunningState;

    CHIP_ERROR err = LogEvent(event, endpoint, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Failed to emit RunningStateChange event: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void EmitRunningModeChangeEvent(EndpointId endpoint, Optional<ThermostatRunningModeEnum> previousRunningMode,
                                ThermostatRunningModeEnum currentRunningMode)
{
    Events::RunningModeChange::Type event;
    EventNumber eventNumber;

    event.previousRunningMode = previousRunningMode;
    event.currentRunningMode  = currentRunningMode;

    CHIP_ERROR err = LogEvent(event, endpoint, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Failed to emit RunningModeChange event: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void EmitActiveScheduleChangeEvent(EndpointId endpoint, Optional<DataModel::Nullable<chip::ByteSpan>> previousScheduleHandle,
                                   DataModel::Nullable<chip::ByteSpan> currentScheduleHandle)
{
    Events::ActiveScheduleChange::Type event;
    EventNumber eventNumber;

    event.previousScheduleHandle = previousScheduleHandle;
    event.currentScheduleHandle  = currentScheduleHandle;

    CHIP_ERROR err = LogEvent(event, endpoint, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Failed to emit ActiveScheduleChange event: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void EmitActivePresetChangeEvent(EndpointId endpoint, Optional<DataModel::Nullable<chip::ByteSpan>> previousPresetHandle,
                                 DataModel::Nullable<chip::ByteSpan> currentPresetHandle)
{
    Events::ActivePresetChange::Type event;
    EventNumber eventNumber;

    event.previousPresetHandle = previousPresetHandle;
    event.currentPresetHandle  = currentPresetHandle;

    CHIP_ERROR err = LogEvent(event, endpoint, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Failed to emit ActivePresetChange event: %" CHIP_ERROR_FORMAT, err.Format());
    }
}
