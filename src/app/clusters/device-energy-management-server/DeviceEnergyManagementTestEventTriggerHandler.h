/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <app/TestEventTriggerDelegate.h>

/**
 * @brief User handler for handling the test event trigger
 *
 * @note If TestEventTrigger is enabled, it needs to be implemented in the app
 *
 * @param eventTrigger Event trigger to handle
 *
 * @retval true on success
 * @retval false if error happened
 */
bool HandleDeviceEnergyManagementTestEventTrigger(uint64_t eventTrigger);

namespace chip {

/*
 * These Test EventTrigger values can be used to produce artificial DEM forecasts
 *
 * They are sent along with the enableKey (manufacturer defined secret)
 * in the General Diagnostic cluster TestEventTrigger command
 */
enum class DeviceEnergyManagementTrigger : uint64_t
{
    // Simulate a fixed forecast power usage including one or more PowerAdjustmentStructs
    kPowerAdjustment = 0x0098'0000'0000'0000,
    // Clear the PowerAdjustment structs
    kPowerAdjustmentClear = 0x0098'0000'0000'0001,
    // Simulate user opt-out of Local Optimization
    kUserOptOutLocalOptimization = 0x0098'0000'0000'0002,
    // Simulate user opt-out of Grid Optimization
    kUserOptOutGridOptimization = 0x0098'0000'0000'0003,
    // Remove all user opt-out opting out
    kUserOptOutClearAll = 0x0098'0000'0000'0004,
    // Simulate a fixed forecast with EarliestStartTime earlier than startTime, and LatestEndTime greater than EndTime
    kStartTimeAdjustment = 0x0098'0000'0000'0005,
    // Clear the StartTimeAdjustment simulated forecast
    kStartTimeAdjustmentClear = 0x0098'0000'0000'0006,
    // Simulate a fixed forecast with one pausable slo with MinPauseDuration >1, MaxPauseDuration>1 and one non pausable slot
    kPausable = 0x0098'0000'0000'0007,
    // Simulate a moving time to the next forecast slot
    kPausableNextSlot = 0x0098'0000'0000'0008,
    // Clear the Pausable simulated forecast
    kPausableClear = 0x0098'0000'0000'0009,
    // Simulate a forecast power usage with at least 2 and at most 4 slots
    kForecastAdjustment = 0x0098'0000'0000'000A,
    // Simulate moving time to the next forecast slot
    kForecastAdjustmentNextSlot = 0x0098'0000'0000'000B,
    // Clear the forecast adjustment
    kForecastAdjustmentClear = 0x0098'0000'0000'000C,
    // Simulate a forecast power usage with at least 2 and at most 4 slots
    kConstraintBasedAdjustment = 0x0098'0000'0000'000D,
    //  Clear the constraint based adjustment
    kConstraintBasedAdjustmentClear = 0x0098'0000'0000'000E,
    // Simulate a forecast with at least 1 slot
    kForecast = 0x0098'0000'0000'000F,
    // Clear the forecast
    kForecastClear = 0x0098'0000'0000'0010,
};

class DeviceEnergyManagementTestEventTriggerHandler : public TestEventTriggerHandler
{
public:
    DeviceEnergyManagementTestEventTriggerHandler() {}

    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override
    {
        if (HandleDeviceEnergyManagementTestEventTrigger(eventTrigger))
        {
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
};

} // namespace chip
