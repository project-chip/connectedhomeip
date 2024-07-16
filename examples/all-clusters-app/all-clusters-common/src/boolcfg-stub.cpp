/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/clusters/boolean-state-configuration-server/BooleanStateConfigurationTestEventTriggerHandler.h>
#include <app/clusters/boolean-state-configuration-server/boolean-state-configuration-server.h>

#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app::Clusters::BooleanStateConfiguration;
using namespace chip::DeviceLayer;

bool HandleBooleanStateConfigurationTestEventTrigger(uint64_t eventTrigger)
{
    BooleanStateConfigurationTrigger trigger = static_cast<BooleanStateConfigurationTrigger>(eventTrigger);

    switch (trigger)
    {
    case BooleanStateConfigurationTrigger::kSensorTrigger:
        ChipLogProgress(Support, "[BooleanStateConfiguration-Test-Event] => Trigger sensor");
        SetAllEnabledAlarmsActive(1);
        break;

    case BooleanStateConfigurationTrigger::kSensorUntrigger:
        ChipLogProgress(Support, "[BooleanStateConfiguration-Test-Event] => Untrigger sensor");
        ClearAllAlarms(1);
        break;

    default:

        return false;
    }

    return true;
}
