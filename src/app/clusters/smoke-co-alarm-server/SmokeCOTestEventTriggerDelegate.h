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

namespace chip {

enum class SmokeCOTrigger : uint64_t
{
    // Force alarm commands
    kForceSmokeWarning           = 0xffffffff00000090,
    kForceCOWarning              = 0xffffffff00000091,
    kForceSmokeInterconnect      = 0xffffffff00000092,
    kForceMalfunction            = 0xffffffff00000093,
    kForceCOInterconnect         = 0xffffffff00000094,
    kForceLowBatteryWarning      = 0xffffffff00000095,
    kForceSmokeContaminationHigh = 0xffffffff00000096,
    kForceSmokeContaminationLow  = 0xffffffff00000097,
    kForceSmokeSensitivityHigh   = 0xffffffff00000098,
    kForceSmokeSensitivityLow    = 0xffffffff00000099,
    kForceEndOfLife              = 0xffffffff0000009a,
    kForceSilence                = 0xffffffff0000009b,
    kForceSmokeCritical          = 0xffffffff0000009c,
    kForceCOCritical             = 0xffffffff0000009d,
    kForceLowBatteryCritical     = 0xffffffff0000009e,
    // Clear alarm commands
    kClearSmoke             = 0xffffffff000000a0,
    kClearCO                = 0xffffffff000000a1,
    kClearSmokeInterconnect = 0xffffffff000000a2,
    kClearMalfunction       = 0xffffffff000000a3,
    kClearCOInterconnect    = 0xffffffff000000a4,
    kClearBatteryLevelLow   = 0xffffffff000000a5,
    kClearContamination     = 0xffffffff000000a6,
    kClearSensitivity       = 0xffffffff000000a8,
    kClearEndOfLife         = 0xffffffff000000aa,
    kClearSilence           = 0xffffffff000000ab
};

class SmokeCOTestEventTriggerDelegate : public TestEventTriggerDelegate
{
public:
    explicit SmokeCOTestEventTriggerDelegate(const ByteSpan & enableKey, TestEventTriggerDelegate * otherDelegate) :
        mEnableKey(enableKey), mOtherDelegate(otherDelegate)
    {}

    bool DoesEnableKeyMatch(const ByteSpan & enableKey) const override;
    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override;

private:
    ByteSpan mEnableKey;
    TestEventTriggerDelegate * mOtherDelegate;
};

} // namespace chip

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
