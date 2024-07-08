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
bool HandleWaterHeaterManagementTestEventTrigger(uint64_t eventTrigger);

namespace chip {

/*
 * These Test EventTrigger values can be used to produce artificial water heater configuration
 * and water temperatures.
 *
 * They are sent along with the enableKey (manufacturer defined secret)
 * in the General Diagnostic cluster TestEventTrigger command
 */
enum class WaterHeaterManagementTrigger : uint64_t
{
    // Simulate installation in a 100L tank full of water at 20C, with a target temperature of 60C, in OFF mode
    kBasicInstallationTestEvent = 0x0094'0000'0000'0000,

    // End simulation of installation
    kBasicInstallationTestEventClear = 0x0094'0000'0000'0001,

    // Simulate 100% of the water in the tank being at 20C
    kWaterTemperature20CTestEvent = 0x0094'0000'0000'0002,

    // Simulate 100% of the water in the tank being at 61C
    kWaterTemperature61CTestEvent = 0x0094'0000'0000'0003,

    // Simulate 100% of the water in the tank being at 66C
    kWaterTemperature66CTestEvent = 0x0094'0000'0000'0004,

    // Simulate the Water Heater Mode being set to MANUAL
    kManualModeTestEvent = 0x0094'0000'0000'0005,

    // Simulate the Water Heater Mode being set to OFF
    kOffModeTestEvent = 0x0094'0000'0000'0006,

    // Simulate drawing off 25% of the tank volume of hot water, replaced with water at 20C
    kDrawOffHotWaterTestEvent = 0x0094'0000'0000'0007,
};

class WaterHeaterManagementTestEventTriggerHandler : public TestEventTriggerHandler
{
public:
    WaterHeaterManagementTestEventTriggerHandler() {}

    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override
    {
        if (HandleWaterHeaterManagementTestEventTrigger(eventTrigger))
        {
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
};

} // namespace chip
