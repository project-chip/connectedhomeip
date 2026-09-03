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

#include <app/TestEventTriggerDelegate.h>

#include <cstdint>

/**
 * Application-provided handler for Electrical Alarm (0x00A1) test-event-triggers.
 *
 * The example app implements this to drive alarm State (and thus the Notify event) for
 * TC-ESALM-3.x, which tests the Notify event lifecycle. Returns true if the trigger was
 * recognized and handled.
 */
bool HandleElectricalAlarmTestEventTrigger(uint64_t eventTrigger);

namespace chip {

/// EventTrigger codes for the Electrical Alarm cluster. The top two bytes carry the cluster id
/// (0x00A1) to namespace the trigger; the low bytes select the alarm bit to activate, or 0 to
/// clear all active alarms.
enum class ElectricalAlarmTrigger : uint64_t
{
    kClearAll          = 0x00a1'0000'0000'0000,
    kSetOverVoltage    = 0x00a1'0000'0000'0001,
    kSetUnderVoltage   = 0x00a1'0000'0000'0002,
    kSetOverFrequency  = 0x00a1'0000'0000'0003,
    kSetUnderFrequency = 0x00a1'0000'0000'0004,
    kSetOverPower      = 0x00a1'0000'0000'0005,
    kSetUnderPower     = 0x00a1'0000'0000'0006,
    kSetOverCurrent    = 0x00a1'0000'0000'0007,
    kSetUnderCurrent   = 0x00a1'0000'0000'0008,
    kSetPowerImport    = 0x00a1'0000'0000'0009,
    kSetPowerExport    = 0x00a1'0000'0000'000a,
};

class ElectricalAlarmTestEventTriggerHandler : public TestEventTriggerHandler
{
public:
    ElectricalAlarmTestEventTriggerHandler() = default;

    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override
    {
        eventTrigger = clearEndpointInEventTrigger(eventTrigger);
        return HandleElectricalAlarmTestEventTrigger(eventTrigger) ? CHIP_NO_ERROR : CHIP_ERROR_INVALID_ARGUMENT;
    }
};

} // namespace chip
