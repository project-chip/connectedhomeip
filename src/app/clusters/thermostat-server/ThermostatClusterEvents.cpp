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

#include "ThermostatClusterBase.h"
#include <app/EventLogging.h>
#include <app/server-cluster/ServerClusterContext.h>

#include <limits>

using namespace chip::app::Clusters::Thermostat::Attributes;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

void ThermostatClusterBase::GenerateSystemModeChangeEvent(Optional<SystemModeEnum> previousSystemMode,
                                                          SystemModeEnum currentSystemMode)
{
    if (!mFeatures.Has(Feature::kEvents))
    {
        return;
    }
    Events::SystemModeChange::Type event;

    event.previousSystemMode = previousSystemMode;
    event.currentSystemMode  = currentSystemMode;

    if (mContext != nullptr)
    {
        mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
        return;
    }

    EventNumber eventNumber;
    CHIP_ERROR err = LogEvent(event, mPath.mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Failed to generate SystemModeChange event: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void ThermostatClusterBase::GenerateLocalTemperatureChangeEvent(DataModel::Nullable<int16_t> currentLocalTemperature)
{
    if (!mFeatures.Has(Feature::kEvents))
    {
        return;
    }
    Events::LocalTemperatureChange::Type event;

    event.currentLocalTemperature = currentLocalTemperature;

    if (mContext != nullptr)
    {
        mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
        return;
    }

    EventNumber eventNumber;
    CHIP_ERROR err = LogEvent(event, mPath.mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Failed to generate LocalTemperatureChange event: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void ThermostatClusterBase::GenerateOccupancyChangeEvent(Optional<BitMask<OccupancyBitmap>> previousOccupancy,
                                                         BitMask<OccupancyBitmap> currentOccupancy)
{
    if (!mFeatures.Has(Feature::kEvents))
    {
        return;
    }
    Events::OccupancyChange::Type event;

    event.previousOccupancy = previousOccupancy;
    event.currentOccupancy  = currentOccupancy;

    if (mContext != nullptr)
    {
        mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
        return;
    }

    EventNumber eventNumber;
    CHIP_ERROR err = LogEvent(event, mPath.mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Failed to generate OccupancyChange event: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void ThermostatClusterBase::GenerateSetpointChangeEvent(SystemModeEnum systemMode, BitMask<OccupancyBitmap> occupancy,
                                                        Optional<temperature> previousSetpoint, temperature currentSetpoint)
{
    if (!mFeatures.Has(Feature::kEvents))
    {
        return;
    }
    Events::SetpointChange::Type event;

    event.systemMode       = systemMode;
    event.occupancy        = MakeOptional(occupancy);
    event.previousSetpoint = previousSetpoint;
    event.currentSetpoint  = currentSetpoint;

    if (mContext != nullptr)
    {
        mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
        return;
    }

    EventNumber eventNumber;
    CHIP_ERROR err = LogEvent(event, mPath.mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Failed to generate SetpointChange event: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void ThermostatClusterBase::GenerateRunningStateChangeEvent(Optional<BitMask<RelayStateBitmap>> previousRunningState,
                                                            BitMask<RelayStateBitmap> currentRunningState)
{
    if (!mFeatures.Has(Feature::kEvents))
    {
        return;
    }
    Events::RunningStateChange::Type event;

    event.previousRunningState = previousRunningState;
    event.currentRunningState  = currentRunningState;

    if (mContext != nullptr)
    {
        mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
        return;
    }

    EventNumber eventNumber;
    CHIP_ERROR err = LogEvent(event, mPath.mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Failed to generate RunningStateChange event: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void ThermostatClusterBase::GenerateRunningModeChangeEvent(Optional<ThermostatRunningModeEnum> previousRunningMode,
                                                           ThermostatRunningModeEnum currentRunningMode)
{
    if (!mFeatures.Has(Feature::kEvents))
    {
        return;
    }
    Events::RunningModeChange::Type event;

    event.previousRunningMode = previousRunningMode;
    event.currentRunningMode  = currentRunningMode;

    if (mContext != nullptr)
    {
        mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
        return;
    }

    EventNumber eventNumber;
    CHIP_ERROR err = LogEvent(event, mPath.mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Failed to generate RunningModeChange event: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void ThermostatClusterBase::GenerateActiveScheduleChangeEvent(Optional<DataModel::Nullable<ByteSpan>> previousScheduleHandle,
                                                              DataModel::Nullable<ByteSpan> currentScheduleHandle)
{
    if (!mFeatures.Has(Feature::kEvents))
    {
        return;
    }
    Events::ActiveScheduleChange::Type event;

    event.previousScheduleHandle = previousScheduleHandle;
    event.currentScheduleHandle  = currentScheduleHandle;

    if (mContext != nullptr)
    {
        mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
        return;
    }

    EventNumber eventNumber;
    CHIP_ERROR err = LogEvent(event, mPath.mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Failed to generate ActiveScheduleChange event: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void ThermostatClusterBase::GenerateActivePresetChangeEvent(Optional<DataModel::Nullable<ByteSpan>> previousPresetHandle,
                                                            DataModel::Nullable<ByteSpan> currentPresetHandle)
{
    if (!mFeatures.Has(Feature::kEvents))
    {
        return;
    }
    Events::ActivePresetChange::Type event;

    event.previousPresetHandle = previousPresetHandle;
    event.currentPresetHandle  = currentPresetHandle;

    if (mContext != nullptr)
    {
        mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
        return;
    }

    EventNumber eventNumber;
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
