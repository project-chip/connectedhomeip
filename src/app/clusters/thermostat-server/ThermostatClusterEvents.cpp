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

#include <app-common/zap-generated/cluster-objects.h>

#include "ThermostatCluster.h"
#include <app/EventLogging.h>

#include <limits>

using namespace chip::app::Clusters::Thermostat::Attributes;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

void ThermostatCluster::GenerateSetpointEvent(AttributeId attributeId, temperature oldTemp, temperature newTemp)
{
    switch (attributeId)
    {
    case OccupiedHeatingSetpoint::Id:
        GenerateSetpointChangeEvent(SystemModeEnum::kHeat, OccupancyBitmap::kOccupied, MakeOptional(oldTemp), newTemp);
        break;
    case UnoccupiedHeatingSetpoint::Id:
        GenerateSetpointChangeEvent(SystemModeEnum::kHeat, BitMask<OccupancyBitmap>(), MakeOptional(oldTemp), newTemp);
        break;
    case OccupiedCoolingSetpoint::Id:
        GenerateSetpointChangeEvent(SystemModeEnum::kCool, OccupancyBitmap::kOccupied, MakeOptional(oldTemp), newTemp);
        break;
    case UnoccupiedCoolingSetpoint::Id:
        GenerateSetpointChangeEvent(SystemModeEnum::kCool, BitMask<OccupancyBitmap>(), MakeOptional(oldTemp), newTemp);
        break;
    }
}

void ThermostatCluster::GenerateSystemModeChangeEvent(Optional<SystemModeEnum> previousSystemMode, SystemModeEnum currentSystemMode)
{
    Events::SystemModeChange::Type event;
    EventNumber eventNumber;

    event.previousSystemMode = previousSystemMode;
    event.currentSystemMode  = currentSystemMode;

    CHIP_ERROR err = LogEvent(event, mPath.mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Failed to generate SystemModeChange event: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void ThermostatCluster::GenerateLocalTemperatureChangeEvent(DataModel::Nullable<int16_t> currentLocalTemperature)
{
    Events::LocalTemperatureChange::Type event;
    EventNumber eventNumber;

    event.currentLocalTemperature = currentLocalTemperature;

    CHIP_ERROR err = LogEvent(event, mPath.mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Failed to generate LocalTemperatureChange event: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void ThermostatCluster::GenerateOccupancyChangeEvent(Optional<BitMask<OccupancyBitmap>> previousOccupancy,
                                                     BitMask<OccupancyBitmap> currentOccupancy)
{
    Events::OccupancyChange::Type event;
    EventNumber eventNumber;

    event.previousOccupancy = previousOccupancy;
    event.currentOccupancy  = currentOccupancy;

    CHIP_ERROR err = LogEvent(event, mPath.mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Failed to generate OccupancyChange event: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void ThermostatCluster::GenerateSetpointChangeEvent(SystemModeEnum systemMode, BitMask<OccupancyBitmap> occupancy,
                                                     Optional<temperature> previousSetpoint, temperature currentSetpoint)
{
    Events::SetpointChange::Type event;
    EventNumber eventNumber;

    event.systemMode       = systemMode;
    event.occupancy        = MakeOptional(occupancy);
    event.previousSetpoint = previousSetpoint;
    event.currentSetpoint  = currentSetpoint;

    CHIP_ERROR err = LogEvent(event, mPath.mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Failed to generate SetpointChange event: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void ThermostatCluster::GenerateRunningStateChangeEvent(Optional<BitMask<RelayStateBitmap>> previousRunningState,
                                                        BitMask<RelayStateBitmap> currentRunningState)
{
    Events::RunningStateChange::Type event;
    EventNumber eventNumber;

    event.previousRunningState = previousRunningState;
    event.currentRunningState  = currentRunningState;

    CHIP_ERROR err = LogEvent(event, mPath.mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Failed to generate RunningStateChange event: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void ThermostatCluster::GenerateRunningModeChangeEvent(Optional<ThermostatRunningModeEnum> previousRunningMode,
                                                       ThermostatRunningModeEnum currentRunningMode)
{
    Events::RunningModeChange::Type event;
    EventNumber eventNumber;

    event.previousRunningMode = previousRunningMode;
    event.currentRunningMode  = currentRunningMode;

    CHIP_ERROR err = LogEvent(event, mPath.mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Failed to generate RunningModeChange event: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void ThermostatCluster::GenerateActiveScheduleChangeEvent(Optional<DataModel::Nullable<ByteSpan>> previousScheduleHandle,
                                                          DataModel::Nullable<ByteSpan> currentScheduleHandle)
{
    Events::ActiveScheduleChange::Type event;
    EventNumber eventNumber;

    event.previousScheduleHandle = previousScheduleHandle;
    event.currentScheduleHandle  = currentScheduleHandle;

    CHIP_ERROR err = LogEvent(event, mPath.mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Failed to generate ActiveScheduleChange event: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void ThermostatCluster::GenerateActivePresetChangeEvent(Optional<DataModel::Nullable<ByteSpan>> previousPresetHandle,
                                                        DataModel::Nullable<ByteSpan> currentPresetHandle)
{
    Events::ActivePresetChange::Type event;
    EventNumber eventNumber;

    event.previousPresetHandle = previousPresetHandle;
    event.currentPresetHandle  = currentPresetHandle;

    CHIP_ERROR err = LogEvent(event, mPath.mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Failed to generate ActivePresetChange event: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
