/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <DEMDelegate.h>
#include <DeviceEnergyManagementDelegateImpl.h>
#include <EVSEManufacturerImpl.h>
#include <app/clusters/device-energy-management-server/DeviceEnergyManagementTestEventTriggerHandler.h>

#include <EnergyTimeUtils.h>

#include "FakeReadings.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DeviceEnergyManagement;

static constexpr uint16_t MAX_SLOTS             = 10;
static constexpr uint16_t MAX_POWER_ADJUSTMENTS = 5;

static chip::app::Clusters::DeviceEnergyManagement::Structs::SlotStruct::Type sSlots[MAX_SLOTS];
static chip::app::Clusters::DeviceEnergyManagement::Structs::ForecastStruct::Type sForecastStruct;
static chip::app::DataModel::Nullable<chip::app::Clusters::DeviceEnergyManagement::Structs::ForecastStruct::Type> sForecast;

static chip::app::Clusters::DeviceEnergyManagement::Structs::PowerAdjustStruct::Type sPowerAdjustments[MAX_POWER_ADJUSTMENTS];
static chip::app::Clusters::DeviceEnergyManagement::Structs::PowerAdjustCapabilityStruct::Type sPowerAdjustCapabilityStruct;
static chip::app::DataModel::Nullable<chip::app::Clusters::DeviceEnergyManagement::Structs::PowerAdjustCapabilityStruct::Type>
    sPowerAdjustmentCapability;

CHIP_ERROR ConfigureForecast(uint16_t numSlots)
{
    uint32_t chipEpoch = 0;

    CHIP_ERROR err = GetEpochTS(chipEpoch);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Support, "ConfigureForecast could not get time");
        return err;
    }

    // planned start time, in UTC, for the entire Forecast. Allow to be a little
    // time in the future as forecastStruct.startTime is used in some tests.
    sForecastStruct.startTime = chipEpoch + 60;

    // earliest start time, in UTC, that the entire Forecast can be shifted to. null value indicates that it can be started
    // immediately.
    sForecastStruct.earliestStartTime = MakeOptional(DataModel::MakeNullable(chipEpoch));

    // planned end time, in UTC, for the entire Forecast.
    sForecastStruct.endTime = chipEpoch * 3;

    // latest end time, in UTC, for the entire Forecast
    sForecastStruct.latestEndTime = MakeOptional(chipEpoch * 3);

    sForecastStruct.isPausable = true;

    sForecastStruct.activeSlotNumber.SetNonNull(0);

    sSlots[0].minDuration       = 10;
    sSlots[0].maxDuration       = 20;
    sSlots[0].defaultDuration   = 15;
    sSlots[0].elapsedSlotTime   = 0;
    sSlots[0].remainingSlotTime = 0;

    sSlots[0].slotIsPausable.SetValue(true);
    sSlots[0].minPauseDuration.SetValue(10);
    sSlots[0].maxPauseDuration.SetValue(60);

    if (GetDEMDelegate()->HasFeature(DeviceEnergyManagement::Feature::kPowerForecastReporting))
    {
        sSlots[0].nominalPower.SetValue(2500000);
        sSlots[0].minPower.SetValue(1200000);
        sSlots[0].maxPower.SetValue(7600000);
    }

    sSlots[0].nominalEnergy.SetValue(2000);

    if (GetDEMDelegate()->HasFeature(DeviceEnergyManagement::Feature::kStateForecastReporting))
    {
        sSlots[0].manufacturerESAState.SetValue(23);
    }

    for (uint16_t slotNo = 1; slotNo < numSlots; slotNo++)
    {
        sSlots[slotNo].minDuration       = 2 * sSlots[slotNo - 1].minDuration;
        sSlots[slotNo].maxDuration       = 2 * sSlots[slotNo - 1].maxDuration;
        sSlots[slotNo].defaultDuration   = 2 * sSlots[slotNo - 1].defaultDuration;
        sSlots[slotNo].elapsedSlotTime   = 2 * sSlots[slotNo - 1].elapsedSlotTime;
        sSlots[slotNo].remainingSlotTime = 2 * sSlots[slotNo - 1].remainingSlotTime;

        // Need slotNo == 1 not to be pausible for test DEM 2.4 step 3b
        sSlots[slotNo].slotIsPausable.SetValue((slotNo & 1) == 0 ? true : false);
        sSlots[slotNo].minPauseDuration.SetValue(2 * sSlots[slotNo - 1].slotIsPausable.Value());
        sSlots[slotNo].maxPauseDuration.SetValue(2 * sSlots[slotNo - 1].maxPauseDuration.Value());

        if (GetDEMDelegate()->HasFeature(DeviceEnergyManagement::Feature::kPowerForecastReporting))
        {
            sSlots[slotNo].nominalPower.SetValue(sSlots[slotNo - 1].nominalPower.Value());
            sSlots[slotNo].minPower.SetValue(sSlots[slotNo - 1].minPower.Value());
            sSlots[slotNo].maxPower.SetValue(sSlots[slotNo - 1].maxPower.Value());

            sSlots[slotNo].nominalEnergy.SetValue(2 * sSlots[slotNo - 1].nominalEnergy.Value());
        }

        if (GetDEMDelegate()->HasFeature(DeviceEnergyManagement::Feature::kStateForecastReporting))
        {
            sSlots[slotNo].manufacturerESAState.SetValue(sSlots[slotNo - 1].manufacturerESAState.Value() + 1);
        }
    }

    sForecastStruct.slots = DataModel::List<const DeviceEnergyManagement::Structs::SlotStruct::Type>(sSlots, numSlots);

    GetDEMDelegate()->SetForecast(DataModel::MakeNullable(sForecastStruct));

    return CHIP_NO_ERROR;
}

void SetTestEventTrigger_PowerAdjustment()
{
    sPowerAdjustments[0].minPower    = 5000 * 1000;  // 5kW
    sPowerAdjustments[0].maxPower    = 30000 * 1000; // 30kW
    sPowerAdjustments[0].minDuration = 10;           // 30s
    sPowerAdjustments[0].maxDuration = 60;           // 60s

    DataModel::List<const DeviceEnergyManagement::Structs::PowerAdjustStruct::Type> powerAdjustmentList(sPowerAdjustments, 1);

    sPowerAdjustCapabilityStruct.cause = PowerAdjustReasonEnum::kNoAdjustment;
    sPowerAdjustCapabilityStruct.powerAdjustCapability.SetNonNull(powerAdjustmentList);
    sPowerAdjustmentCapability.SetNonNull(sPowerAdjustCapabilityStruct);

    CHIP_ERROR err = GetDEMDelegate()->SetPowerAdjustmentCapability(sPowerAdjustmentCapability);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Support, "SetTestEventTrigger_PowerAdjustment failed %s", chip::ErrorStr(err));
    }
}

void SetTestEventTrigger_ClearForecast()
{
    sPowerAdjustments[0].minPower    = 0;
    sPowerAdjustments[0].maxPower    = 0;
    sPowerAdjustments[0].minDuration = 0;
    sPowerAdjustments[0].maxDuration = 0;

    DataModel::List<const DeviceEnergyManagement::Structs::PowerAdjustStruct::Type> powerAdjustmentList(sPowerAdjustments, 1);

    sPowerAdjustCapabilityStruct.powerAdjustCapability.SetNonNull(powerAdjustmentList);
    sPowerAdjustCapabilityStruct.cause = PowerAdjustReasonEnum::kNoAdjustment;

    DataModel::Nullable<DeviceEnergyManagement::Structs::PowerAdjustCapabilityStruct::Type> powerAdjustmentCapabilityStruct(
        sPowerAdjustCapabilityStruct);

    CHIP_ERROR err = GetDEMDelegate()->SetPowerAdjustmentCapability(powerAdjustmentCapabilityStruct);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Support, "SetTestEventTrigger_PowerAdjustment failed %s", chip::ErrorStr(err));
    }
}

void SetTestEventTrigger_StartTimeAdjustment()
{
    ConfigureForecast(2);

    // Get the current forecast ad update the earliestStartTime and latestEndTime
    sForecastStruct = GetDEMDelegate()->GetForecast().Value();

    uint32_t chipEpoch = 0;

    CHIP_ERROR err = GetEpochTS(chipEpoch);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Support, "ConfigureForecast_EarliestStartLatestEndTimes could not get time");
    }

    // planned start time, in UTC, for the entire Forecast.
    sForecastStruct.startTime = chipEpoch;

    // Set the earliest start time, in UTC, to that before the startTime
    sForecastStruct.earliestStartTime = Optional<DataModel::Nullable<uint32_t>>{ DataModel::Nullable<uint32_t>{ chipEpoch - 60 } };

    // Planned end time, in UTC, for the entire Forecast.
    sForecastStruct.endTime = chipEpoch * 3;

    // Latest end time, in UTC, for the entire Forecast which is > sForecastStruct.endTime
    sForecastStruct.latestEndTime = Optional<uint32_t>(chipEpoch * 3 + 60);

    GetDEMDelegate()->SetForecast(DataModel::MakeNullable(sForecastStruct));
}

void SetTestEventTrigger_StartTimeAdjustmentClear()
{
    // Get the current forecast ad update the earliestStartTime and latestEndTime
    sForecastStruct = GetDEMDelegate()->GetForecast().Value();

    sForecastStruct.startTime = static_cast<uint32_t>(0);
    sForecastStruct.endTime   = static_cast<uint32_t>(0);

    sForecastStruct.earliestStartTime = NullOptional;
    sForecastStruct.latestEndTime     = NullOptional;

    GetDEMDelegate()->SetForecast(DataModel::MakeNullable(sForecastStruct));
}

void SetTestEventTrigger_UserOptOutOptimization(OptOutStateEnum optOutState)
{
    GetDEMDelegate()->SetOptOutState(optOutState);
}

void SetTestEventTrigger_Pausable()
{
    ConfigureForecast(2);
}

void SetTestEventTrigger_PausableNextSlot()
{
    // Get the current forecast ad update the active slot number
    sForecastStruct = GetDEMDelegate()->GetForecast().Value();
    sForecastStruct.activeSlotNumber.SetNonNull(1);

    GetDEMDelegate()->SetForecast(DataModel::MakeNullable(sForecastStruct));
}

void SetTestEventTrigger_Forecast()
{
    ConfigureForecast(2);
}

void SetTestEventTrigger_ForecastClear()
{
    sForecastStruct.startTime = 0;
    sForecastStruct.endTime   = 0;
    sForecastStruct.earliestStartTime.ClearValue();
    sForecastStruct.latestEndTime.ClearValue();
    sForecastStruct.isPausable = false;
    sForecastStruct.activeSlotNumber.SetNull();
    sForecastStruct.slots = DataModel::List<const DeviceEnergyManagement::Structs::SlotStruct::Type>();

    GetDEMDelegate()->SetForecast(DataModel::MakeNullable(sForecastStruct));
}

void SetTestEventTrigger_ForecastAdjustment()
{
    ConfigureForecast(2);

    // The following values need to match the equivalent values in src/python_testing/TC_DEM_2_5.py
    sForecastStruct = GetDEMDelegate()->GetForecast().Value();
    sSlots[0].minPowerAdjustment.SetValue(20);
    sSlots[0].maxPowerAdjustment.SetValue(2000);
    sSlots[0].minDurationAdjustment.SetValue(120);
    sSlots[0].maxDurationAdjustment.SetValue(240);

    sForecastStruct.slots = DataModel::List<const DeviceEnergyManagement::Structs::SlotStruct::Type>(sSlots, 2);

    GetDEMDelegate()->SetForecast(DataModel::MakeNullable(sForecastStruct));
}

void SetTestEventTrigger_ForecastAdjustmentNextSlot()
{
    sForecastStruct = GetDEMDelegate()->GetForecast().Value();
    sForecastStruct.activeSlotNumber.SetNonNull(sForecastStruct.activeSlotNumber.Value() + 1);

    GetDEMDelegate()->SetForecast(DataModel::MakeNullable(sForecastStruct));
}

void SetTestEventTrigger_ConstraintBasedAdjustment()
{
    ConfigureForecast(4);
}

bool HandleDeviceEnergyManagementTestEventTrigger(uint64_t eventTrigger)
{
    DeviceEnergyManagementTrigger trigger = static_cast<DeviceEnergyManagementTrigger>(eventTrigger);

    switch (trigger)
    {
    case DeviceEnergyManagementTrigger::kPowerAdjustment:
        ChipLogProgress(
            Support,
            "[PowerAdjustment-Test-Event] => Simulate a fixed forecast power usage including one or more PowerAdjustmentStructs");
        SetTestEventTrigger_PowerAdjustment();
        break;
    case DeviceEnergyManagementTrigger::kPowerAdjustmentClear:
        ChipLogProgress(Support, "[PowerAdjustmentClear-Test-Event] => Clear the PowerAdjustment structs");
        SetTestEventTrigger_ClearForecast();
        break;
    case DeviceEnergyManagementTrigger::kUserOptOutLocalOptimization:
        ChipLogProgress(Support, "[UserOptOutLocalOptimization-Test-Event] => Simulate user opt-out of Local Optimization");
        SetTestEventTrigger_UserOptOutOptimization(OptOutStateEnum::kLocalOptOut);
        break;
    case DeviceEnergyManagementTrigger::kUserOptOutGridOptimization:
        ChipLogProgress(Support, "[UserOptOutGrisOptimization-Test-Event] => Simulate user opt-out of Grid Optimization");
        SetTestEventTrigger_UserOptOutOptimization(OptOutStateEnum::kGridOptOut);
        break;
    case DeviceEnergyManagementTrigger::kUserOptOutClearAll:
        ChipLogProgress(Support, "[UserOptOutClearAll-Test-Event] => Remove all user opt-out opting out");
        SetTestEventTrigger_UserOptOutOptimization(OptOutStateEnum::kNoOptOut);
        break;
    case DeviceEnergyManagementTrigger::kStartTimeAdjustment:
        ChipLogProgress(Support,
                        "[StartTimeAdjustment-Test-Event] => Simulate a fixed forecast with EarliestStartTime earlier than "
                        "startTime, and LatestEndTime greater than EndTime");
        SetTestEventTrigger_StartTimeAdjustment();
        break;
    case DeviceEnergyManagementTrigger::kStartTimeAdjustmentClear:
        ChipLogProgress(Support, "[StartTimeAdjustmentClear-Test-Event] => Clear the StartTimeAdjustment simulated forecast");
        SetTestEventTrigger_StartTimeAdjustmentClear();
        break;
    case DeviceEnergyManagementTrigger::kPausable:
        ChipLogProgress(Support,
                        "[Pausable-Test-Event] => Simulate a fixed forecast with one pausable slot with MinPauseDuration >1, "
                        "MaxPauseDuration>1 and one non pausable slot");
        SetTestEventTrigger_Pausable();
        break;
    case DeviceEnergyManagementTrigger::kPausableNextSlot:
        ChipLogProgress(Support, "[PausableNextSlot-Test-Event] => Simulate a moving time to the next forecast slot");
        SetTestEventTrigger_PausableNextSlot();
        break;
    case DeviceEnergyManagementTrigger::kPausableClear:
        ChipLogProgress(Support, "[PausableClear-Test-Event] => Clear the Pausable simulated forecast");
        SetTestEventTrigger_ClearForecast();
        break;
    case DeviceEnergyManagementTrigger::kForecastAdjustment:
        ChipLogProgress(Support,
                        "[ForecastAdjustment-Test-Event] => Simulate a forecast power usage with at least 2 and at most 4 slots");
        SetTestEventTrigger_ForecastAdjustment();
        break;
    case DeviceEnergyManagementTrigger::kForecastAdjustmentNextSlot:
        ChipLogProgress(Support, "[ForecastAdjustmentNextSlot-Test-Event] => Simulate moving time to the next forecast slot");
        SetTestEventTrigger_ForecastAdjustmentNextSlot();
        break;
    case DeviceEnergyManagementTrigger::kForecastAdjustmentClear:
        ChipLogProgress(Support, "[ForecastAdjustmentClear-Test-Event] => Clear the forecast adjustment");
        SetTestEventTrigger_ClearForecast();
        break;
    case DeviceEnergyManagementTrigger::kConstraintBasedAdjustment:
        ChipLogProgress(
            Support,
            "[ConstraintBasedAdjustment-Test-Event] => Simulate a forecast power usage with at least 2 and at most 4 slots");
        SetTestEventTrigger_ConstraintBasedAdjustment();
        break;
    case DeviceEnergyManagementTrigger::kConstraintBasedAdjustmentClear:
        ChipLogProgress(Support, "[ConstraintBasedAdjustmentClear-Test-Event] => Clear the constraint based adjustment");
        SetTestEventTrigger_ClearForecast();
        break;
    case DeviceEnergyManagementTrigger::kForecast:
        ChipLogProgress(Support, "[Forecast-Test-Event] => Create a forecast with at least 1 slot");
        SetTestEventTrigger_Forecast();
        break;
    case DeviceEnergyManagementTrigger::kForecastClear:
        ChipLogProgress(Support, "[ForecastClear-Test-Event] => Clear the forecast");
        SetTestEventTrigger_ForecastClear();
        break;

    default:
        return false;
    }

    return true;
}
