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
bool HandleEnergyReportingTestEventTrigger(uint64_t eventTrigger);

namespace chip {

/*
 * These Test EventTrigger values can be used to fake meter reading data
 *
 * They are sent along with the enableKey (manufacturer defined secret)
 * in the General Diagnostic cluster TestEventTrigger command
 */
enum class EnergyReportingTrigger : uint64_t
{
    // We use the Electrical Energy Measurement cluster ID as our TE trigger space
    // Stop Fake readings
    kFakeReadingsStop = 0x0091'0000'0000'0000,
    // Fake a load (importing) readings at 1kW 230V 4.34A with 2s updates
    kFakeReadingsLoadStart_1kW_2s = 0x0091'0000'0000'0001,
    // Fake a generator (exporting) readings at 3kW 230V 3.33A with 5s updates
    kFakeReadingsGenStart_3kW_5s = 0x0091'0000'0000'0002,

};

class EnergyReportingTestEventTriggerHandler : public TestEventTriggerHandler
{
public:
    EnergyReportingTestEventTriggerHandler() {}

    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override
    {
        if (HandleEnergyReportingTestEventTrigger(eventTrigger))
        {
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
};

} // namespace chip
