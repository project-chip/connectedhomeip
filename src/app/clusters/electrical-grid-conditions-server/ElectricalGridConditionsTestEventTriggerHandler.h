/*
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
bool HandleElectricalGridConditionsTestEventTrigger(uint64_t eventTrigger);

namespace chip {

/*
 * These Test EventTrigger values can be used to update price information
 *
 * They are sent along with the enableKey (manufacturer defined secret)
 * in the General Diagnostic cluster TestEventTrigger command
 */
enum class ElectricalGridConditionsTrigger : uint64_t
{
    // Simulate an updated CurrentConditions with PeriodStart set to current time.
    // This should cause the CurrentConditions attribute to be updated and a CurrentConditionsChanged
    // event to be sent.
    // Note that this should not update the ForecastConditions attribute or cause the ForecastChange event to be sent.
    kCurrentConditionsUpdate = 0x00A0'0000'0000'0000,
    // Simulate an updated ForecastConditions list to be sent with 48 entries to cover 48 x 30 minute
    // intervals (24 hours) in the future from the next half-hour aligned time.
    // This should cause the ForecastConditions attribute to be updated and a ForecastChanged event
    // to be sent. Note that this should not update the CurrentConditions attribute or
    // cause the CurrentConditionsChanged event to be sent.
    kForecastConditionsUpdate = 0x00A0'0000'0000'0001,
};

class ElectricalGridConditionsTestEventTriggerHandler : public TestEventTriggerHandler
{
public:
    ElectricalGridConditionsTestEventTriggerHandler() {}

    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override
    {
        eventTrigger = clearEndpointInEventTrigger(eventTrigger);
        if (HandleElectricalGridConditionsTestEventTrigger(eventTrigger))
        {
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
};

} // namespace chip
