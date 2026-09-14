/*
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
 * Application-provided handler for Electrical Protection Alarm (0x00A3) test-event-triggers.
 *
 * The example app implements this to drive the alarm State (and thus the Notify event) for
 * TC-EPALM-2.2 (state machine) and TC-EPALM-3.1 (Notify event), which cannot change alarm state
 * over Matter (EPALM accepts no commands). Returns true if the trigger was recognized+handled.
 */
bool HandleElectricalProtectionAlarmTestEventTrigger(uint64_t eventTrigger);

namespace chip {

/// EventTrigger codes for the Electrical Protection Alarm cluster. The top two bytes carry the
/// cluster id (0x00A3) to namespace the trigger. In the low byte, 0x00 clears every alarm, 0x01
/// through 0x07 set a single alarm bit, and 0x11 through 0x17 clear that same bit. The per-alarm
/// clear codes are what TC-EPALM-3.1 through 3.7 use to drive a Notify event back to Inactive.
enum class ElectricalProtectionAlarmTrigger : uint64_t
{
    kClearAll                = 0x00a3'0000'0000'0000,
    kSetShortCircuitFault    = 0x00a3'0000'0000'0001,
    kSetOverLoadFault        = 0x00a3'0000'0000'0002,
    kSetOverVoltageFault     = 0x00a3'0000'0000'0003,
    kSetVoltageSurgeFault    = 0x00a3'0000'0000'0004,
    kSetResidualCurrentFault = 0x00a3'0000'0000'0005,
    kSetArcFault             = 0x00a3'0000'0000'0006,
    kSetSelfTest             = 0x00a3'0000'0000'0007,

    kClearShortCircuitFault    = 0x00a3'0000'0000'0011,
    kClearOverLoadFault        = 0x00a3'0000'0000'0012,
    kClearOverVoltageFault     = 0x00a3'0000'0000'0013,
    kClearVoltageSurgeFault    = 0x00a3'0000'0000'0014,
    kClearResidualCurrentFault = 0x00a3'0000'0000'0015,
    kClearArcFault             = 0x00a3'0000'0000'0016,
    kClearSelfTest             = 0x00a3'0000'0000'0017,
};

class ElectricalProtectionAlarmTestEventTriggerHandler : public TestEventTriggerHandler
{
public:
    ElectricalProtectionAlarmTestEventTriggerHandler() = default;

    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override
    {
        eventTrigger = clearEndpointInEventTrigger(eventTrigger);
        return HandleElectricalProtectionAlarmTestEventTrigger(eventTrigger) ? CHIP_NO_ERROR : CHIP_ERROR_INVALID_ARGUMENT;
    }
};

} // namespace chip
