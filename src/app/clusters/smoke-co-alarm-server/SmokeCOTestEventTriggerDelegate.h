/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#pragma once

#include <app/TestEventTriggerDelegate.h>

namespace chip {
namespace trigger {

// force alarm commands
const uint64_t ForceSmokeWarning           = 0xffffffff00000090;
const uint64_t ForceSmokeInterconnect      = 0xffffffff00000092;
const uint64_t ForceCOInterconnect         = 0xffffffff00000094;
const uint64_t ForceSmokeContaminationHigh = 0xffffffff00000096;
const uint64_t ForceSmokeContaminationLow  = 0xffffffff00000097;
const uint64_t ForceSmokeSensitivityHigh   = 0xffffffff00000098;
const uint64_t ForceSmokeSensitivityLow    = 0xffffffff00000099;
const uint64_t ForceSmokeCritical          = 0xffffffff0000009c;

const uint64_t ForceCOWarning  = 0xffffffff00000091;
const uint64_t ForceCOCritical = 0xffffffff0000009d;

const uint64_t ForceMalfunction = 0xffffffff00000093;

const uint64_t ForceLowBatteryWarning  = 0xffffffff00000095;
const uint64_t ForceLowBatteryCritical = 0xffffffff0000009e;

const uint64_t ForceCOEndOfLife = 0xffffffff0000009a;
const uint64_t ForceSilence     = 0xffffffff0000009b;

// clear alarm commands
const uint64_t ClearSmoke             = 0xffffffff000000a0;
const uint64_t ClearCO                = 0xffffffff000000a1;
const uint64_t ClearSmokeInterconnect = 0xffffffff000000a2;
const uint64_t ClearMalfunction       = 0xffffffff000000a3;
const uint64_t ClearCOInterconnect    = 0xffffffff000000a4;
const uint64_t ClearBatteryLevelLow   = 0xffffffff000000a5;
const uint64_t ClearContamination     = 0xffffffff000000a6;
const uint64_t ClearSensitivity       = 0xffffffff000000a8;

const uint64_t ClearCOEndOfLife = 0xffffffff000000aa;
const uint64_t ClearSilence     = 0xffffffff000000ab;
} // namespace trigger
class SmokeCOTestEventTriggerDelegate : public TestEventTriggerDelegate
{
public:
     explicit SmokeCOTestEventTriggerDelegate(const ByteSpan & enableKey) : mEnableKey(enableKey) {}

    bool DoesEnableKeyMatch(const ByteSpan & enableKey) const override;
    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override;
    CHIP_ERROR SetSmokeCODelegate(TestEventTriggerDelegate * scoDelegate);
    // Highest level of Expressed state is Manufacturer specific.
    // This is specifically written in support of certification test TC-2.6
    void ExampleSetExpressedState (void);

private:
    ByteSpan mEnableKey;

};

} // namespace chip
