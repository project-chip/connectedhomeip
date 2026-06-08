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
bool HandleMeterIdentificationTestEventTrigger(uint64_t eventTrigger);

namespace chip {

/*
 * These Test EventTrigger values are specified in the TC_MTRID test plan
 * and are defined conditions used in test events.
 *
 * They are sent along with the enableKey (manufacturer defined secret)
 * in the General Diagnostic cluster TestEventTrigger command
 */
enum class MeterIdentificationTrigger : uint64_t
{
    // Scenarios
    // Attributes Value Update Test Event | Increment all attributes values (strings are incremented lexicographically)
    kAttributesValueUpdate = 0x0b06000000000000,
    // Attributes Value Test Event Clear | Return the device to pre-test status
    kAttributesValueUpdateClear = 0x0b06000000000001,
};

class MeterIdentificationTestEventTriggerHandler : public TestEventTriggerHandler
{
public:
    explicit MeterIdentificationTestEventTriggerHandler() {}

    /** This function must return True if the eventTrigger is recognised and handled
     *  It must return False to allow a higher level TestEvent handler to check other
     *  clusters that may handle it.
     */
    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override
    {
        eventTrigger = clearEndpointInEventTrigger(eventTrigger);
        if (HandleMeterIdentificationTestEventTrigger(eventTrigger))
        {
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
};

} // namespace chip
