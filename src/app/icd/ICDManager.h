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

#include <system/SystemClock.h>

namespace chip {
namespace app {

/**
 * @brief ICD Manager is responsible of processing the events and triggering the correct action for an ICD
 */
class ICDManager
{
    enum OperationalState : uint8_t
    {
        IdleMode,
        ActiveMode,
    };

    enum ICDMode : uint8_t
    {
        SIT, // Short Interval Time ICD
        LIT, // Long Interval Time ICD
    };

public:
    ICDManager();
    void UpdateIcdMode();
    void UpdateOperationStates(OperationalState state);
    ICDMode GetIcdMode() { return mIcdMode; }
    OperationalState GetOperationalState() { return mOperationalState; }

    static System::Clock::Milliseconds32 GetSlowPollingInterval() { return kSlowPollingInterval; }
    static System::Clock::Milliseconds32 GetFastPollingInterval() { return kFastPollingInterval; }

private:
    static constexpr System::Clock::Milliseconds32 kICDSitModePollingThreashold = 15000_ms32;

    // TODO ICD should they be System::Clock::Milliseconds32
    static constexpr System::Clock::Milliseconds32 kSlowPollingInterval = CHIP_DEVICE_CONFIG_ICD_SLOW_POLL_INTERVAL;
    static constexpr System::Clock::Milliseconds32 kFastPollingInterval = CHIP_DEVICE_CONFIG_ICD_FAST_POLL_INTERVAL;

    void OnIdleModeDone(System::Layer * aLayer, void * appState);
    void OnActiveModeDone(System::Layer * aLayer, void * appState);

    OperationalState mOperationalState = IdleMode;
    ICDMode mIcdMode                   = SIT;
};

} // namespace app
} // namespace chip
