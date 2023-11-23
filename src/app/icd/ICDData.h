/**
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

#include <platform/CHIPDeviceConfig.h>
#include <system/SystemClock.h>

#ifndef ICD_ENFORCE_SIT_SLOW_POLL_LIMIT
// Set to 1 to enforce SIT Slow Polling Max value to 15seconds (spec 9.16.1.5)
#define ICD_ENFORCE_SIT_SLOW_POLL_LIMIT 0
#endif

namespace chip {

namespace app {
// Forward declaration of ICDManager to allow it to be friend with ICDData
class ICDManager;
} // namespace app

class ICDData
{
public:
    enum class ICDMode : uint8_t
    {
        SIT, // Short Interval Time ICD
        LIT, // Long Interval Time ICD
    };

    ICDData() = default;

    uint32_t GetIdleModeDurationSec() { return mIdleInterval_s; }

    uint32_t GetActiveModeDurationMs() { return mActiveInterval_ms; }

    uint16_t GetActiveModeThresholdMs() { return mActiveThreshold_ms; }

    uint32_t GetICDCounter() { return mICDCounter; }

    void SetICDCounter(uint32_t count) { mICDCounter = count; }

    uint16_t GetClientsSupportedPerFabric() { return mFabricClientsSupported; }

    System::Clock::Milliseconds32 GetSITPollingThreshold() { return kSITPollingThreshold; }

    System::Clock::Milliseconds32 GetFastPollingInterval() { return kFastPollingInterval; }

    System::Clock::Milliseconds32 GetSlowPollingInterval();

    ICDMode GetICDMode() { return mICDMode; }

private:
    // ICD Operating mode is managed by the ICDManager but stored in the ICDData to enable consummers to access it without creating
    // a circular dependency To avoid allowing consummers changing the state of the ICD mode without going through the ICDManager;
    // making the ICDManager a friend.
    friend class chip::app::ICDManager;

    void SetICDMode(ICDMode mode) { mICDMode = mode; };

    static_assert((CHIP_CONFIG_ICD_IDLE_MODE_DURATION_SEC) <= 64800,
                  "Spec requires the IdleModeDuration to be equal or inferior to 64800s.");
    static_assert((CHIP_CONFIG_ICD_IDLE_MODE_DURATION_SEC) >= 1,
                  "Spec requires the IdleModeDuration to be equal or greater to 1s.");
    uint32_t mIdleInterval_s = CHIP_CONFIG_ICD_IDLE_MODE_DURATION_SEC;

    static_assert((CHIP_CONFIG_ICD_ACTIVE_MODE_DURATION_MS) <= (CHIP_CONFIG_ICD_IDLE_MODE_DURATION_SEC * kMillisecondsPerSecond),
                  "Spec requires the IdleModeDuration be equal or greater to the ActiveModeDuration.");
    uint32_t mActiveInterval_ms = CHIP_CONFIG_ICD_ACTIVE_MODE_DURATION_MS;

    uint16_t mActiveThreshold_ms = CHIP_CONFIG_ICD_ACTIVE_MODE_THRESHOLD_MS;

    // TODO : Implement ICD counter
    // https://github.com/project-chip/connectedhomeip/issues/29184
    uint32_t mICDCounter = 0;

    static_assert((CHIP_CONFIG_ICD_CLIENTS_SUPPORTED_PER_FABRIC) >= 1,
                  "Spec requires the minimum of supported clients per fabric be equal or greater to 1.");
    uint16_t mFabricClientsSupported = CHIP_CONFIG_ICD_CLIENTS_SUPPORTED_PER_FABRIC;

    // SIT ICDs should have a SlowPollingThreshold shorter than or equal to 15s (spec 9.16.1.5)
    static constexpr System::Clock::Milliseconds32 kSITPollingThreshold = System::Clock::Milliseconds32(15000);
    System::Clock::Milliseconds32 kSlowPollingInterval                  = CHIP_DEVICE_CONFIG_ICD_SLOW_POLL_INTERVAL;
    System::Clock::Milliseconds32 kFastPollingInterval                  = CHIP_DEVICE_CONFIG_ICD_FAST_POLL_INTERVAL;

    // Minimal constraint value of the the ICD attributes.
    static constexpr uint32_t kMinIdleModeDuration    = 500;
    static constexpr uint32_t kMinActiveModeDuration  = 300;
    static constexpr uint16_t kMinActiveModeThreshold = 300;

    ICDMode mICDMode = ICDMode::SIT;
};

} // namespace chip
