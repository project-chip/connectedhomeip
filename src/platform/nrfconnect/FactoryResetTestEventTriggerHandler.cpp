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

#include "FactoryResetTestEventTriggerHandler.h"

#include <app/server/Server.h> // nogncheck
#include <platform/ConfigurationManager.h>
#include <platform/PlatformManager.h>
#include <platform/nrfconnect/Reboot.h>

namespace chip {
namespace DeviceLayer {

CHIP_ERROR FactoryResetTestEventTriggerHandler::HandleEventTrigger(uint64_t eventTrigger)
{
    // Clear the endpoint from the event trigger as per the standard pattern
    eventTrigger = clearEndpointInEventTrigger(eventTrigger);

    if (eventTrigger == kFactoryResetTrigger)
    {
        ChipLogProgress(DeviceLayer, "Factory Reset Test Event Trigger MATCHED - initiating factory reset");

        // Use ScheduleFactoryReset to be sure that all data is cleared
        chip::Server::GetInstance().ScheduleFactoryReset();

        return CHIP_NO_ERROR;
    }

    ChipLogProgress(DeviceLayer, "FactoryReset Handler: Trigger NOT matched, returning INVALID_ARGUMENT");
    // If the trigger is not recognized, return an error to allow other handlers to process it
    return CHIP_ERROR_INVALID_ARGUMENT;
}

} // namespace DeviceLayer
} // namespace chip
