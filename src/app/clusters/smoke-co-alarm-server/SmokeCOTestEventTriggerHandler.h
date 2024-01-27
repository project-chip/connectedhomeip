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
bool HandleSmokeCOTestEventTrigger(uint64_t eventTrigger);

namespace chip {

enum class SmokeCOTrigger : uint64_t
{
    // Force alarm commands
    kForceSmokeWarning           = 0x005c'0000'00000090,
    kForceCOWarning              = 0x005c'0000'00000091,
    kForceSmokeInterconnect      = 0x005c'0000'00000092,
    kForceMalfunction            = 0x005c'0000'00000093,
    kForceCOInterconnect         = 0x005c'0000'00000094,
    kForceLowBatteryWarning      = 0x005c'0000'00000095,
    kForceSmokeContaminationHigh = 0x005c'0000'00000096,
    kForceSmokeContaminationLow  = 0x005c'0000'00000097,
    kForceSmokeSensitivityHigh   = 0x005c'0000'00000098,
    kForceSmokeSensitivityLow    = 0x005c'0000'00000099,
    kForceEndOfLife              = 0x005c'0000'0000009a,
    kForceSilence                = 0x005c'0000'0000009b,
    kForceSmokeCritical          = 0x005c'0000'0000009c,
    kForceCOCritical             = 0x005c'0000'0000009d,
    kForceLowBatteryCritical     = 0x005c'0000'0000009e,
    // Clear alarm commands
    kClearSmoke             = 0x005c'0000'000000a0,
    kClearCO                = 0x005c'0000'000000a1,
    kClearSmokeInterconnect = 0x005c'0000'000000a2,
    kClearMalfunction       = 0x005c'0000'000000a3,
    kClearCOInterconnect    = 0x005c'0000'000000a4,
    kClearBatteryLevelLow   = 0x005c'0000'000000a5,
    kClearContamination     = 0x005c'0000'000000a6,
    kClearSensitivity       = 0x005c'0000'000000a8,
    kClearEndOfLife         = 0x005c'0000'000000aa,
    kClearSilence           = 0x005c'0000'000000ab
};

class SmokeCOTestEventTriggerHandler : public TestEventTriggerHandler
{
public:
    SmokeCOTestEventTriggerHandler() = default;
    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override
    {
        return HandleSmokeCOTestEventTrigger(eventTrigger) ? CHIP_NO_ERROR : CHIP_ERROR_INVALID_ARGUMENT;
    }
};

} // namespace chip
