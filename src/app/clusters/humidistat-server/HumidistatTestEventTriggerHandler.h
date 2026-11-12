/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
bool HandleHumidistatTestEventTrigger(uint64_t eventTrigger);

namespace chip {

/*
 * These Test EventTrigger values are specified for Humidistat (cluster 0x0205)
 * certification testing and are sent with the manufacturer enable key through
 * General Diagnostics TestEventTrigger.
 */
enum class HumidistatTrigger : uint64_t
{
    // DisallowContinuous Event
    kDisallowContinuous = 0x0205000000000000,
    // AllowContinuous Event
    kAllowContinuous = 0x0205000000000001,
    // DisallowSleep Event
    kDisallowSleep = 0x0205000000000002,
    // AllowSleep Event
    kAllowSleep = 0x0205000000000003,
    // DisallowOptimal Event
    kDisallowOptimal = 0x0205000000000004,
    // AllowOptimal Event
    kAllowOptimal = 0x0205000000000005,
};

class HumidistatTestEventTriggerHandler : public TestEventTriggerHandler
{
public:
    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override
    {
        eventTrigger = clearEndpointInEventTrigger(eventTrigger);
        if (HandleHumidistatTestEventTrigger(eventTrigger))
        {
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
};

} // namespace chip
