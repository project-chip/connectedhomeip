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

namespace chip {

namespace app {
// Forward declaration of ICDManager to allow it to be friend with ICDConfigurationData
class ICDManager;
} // namespace app

/**
 * @brief ICDConfigurationData manages and stores ICD related configurations for the ICDManager.
 *        Goal of the class is to expose ICD information to all consummers without creating circular dependencies
 *        since the ICDManager is appart of the App layer
 *
 *        Anyone can read the ICD configurations but only the ICDManager can changes those configurations.
 */
class ICDConfigurationData
{
public:
    static constexpr uint32_t ICD_CHECK_IN_COUNTER_MIN_INCREMENT = 100;

    enum class ICDMode : uint8_t
    {
        SIT, // Short Interval Time ICD
        LIT, // Long Interval Time ICD
    };

    static ICDConfigurationData & GetInstance() { return instance; };

    uint32_t GetIdleModeDurationSec() { return mIdleInterval_s; }

    uint32_t GetActiveModeDurationMs() { return mActiveInterval_ms; }

    uint16_t GetActiveModeThresholdMs() { return mActiveThreshold_ms; }

    uint32_t GetICDCounter() { return mICDCounter; }

    uint16_t GetClientsSupportedPerFabric() { return mFabricClientsSupported; }

    System::Clock::Milliseconds32 GetSITPollingThreshold() { return kSITPollingThreshold; }

    System::Clock::Milliseconds32 GetFastPollingInterval() { return mFastPollingInterval; }

    /**
     * If ICD_ENFORCE_SIT_SLOW_POLL_LIMIT is set to 0, function will always return the configured Slow Polling interval
     * (CHIP_DEVICE_CONFIG_ICD_SLOW_POLL_INTERVAL).
     *
     * If ICD_ENFORCE_SIT_SLOW_POLL_LIMIT is set to 1, the returned value will depend on the devices operating mode.
     * If ICDMode == SIT && the configured slow poll interval is superior to the maximum threshold (15s), the function will return
     * the threshold (15s). If ICDMode == SIT but the configured slow poll interval is equal or inferior to the threshold, the
     * function will the return the configured slow poll interval. If ICDMode == LIT, the function will return the configured slow
     * poll interval.
     *
     * @return System::Clock::Milliseconds32
     */
    System::Clock::Milliseconds32 GetSlowPollingInterval();

    ICDMode GetICDMode() { return mICDMode; }

private:
    // Singleton Object
    ICDConfigurationData() = default;
    static ICDConfigurationData instance;

    // ICD related information is managed by the ICDManager but stored in the ICDConfigurationData to enable consummers to access it
    // without creating a circular dependency. To avoid allowing consummers changing the state of the ICD mode without going through
    // the ICDManager, the ICDManager is a friend that can access the private setters. If a consummer needs to be notified when a
    // value is changed, they can leverage the Observer events the ICDManager generates. See src/app/icd/ICDStateObserver.h
    friend class chip::app::ICDManager;

    void SetICDMode(ICDMode mode) { mICDMode = mode; };
    void SetICDCounter(uint32_t count) { mICDCounter = count; }
    void SetSlowPollingInterval(System::Clock::Milliseconds32 slowPollInterval) { mSlowPollingInterval = slowPollInterval; };
    void SetFastPollingInterval(System::Clock::Milliseconds32 fastPollInterval) { mFastPollingInterval = fastPollInterval; };

    static_assert((CHIP_CONFIG_ICD_IDLE_MODE_DURATION_SEC) <= 64800,
                  "Spec requires the IdleModeDuration to be equal or inferior to 64800s.");
    static_assert((CHIP_CONFIG_ICD_IDLE_MODE_DURATION_SEC) >= 1,
                  "Spec requires the IdleModeDuration to be equal or greater to 1s.");
    uint32_t mIdleInterval_s = CHIP_CONFIG_ICD_IDLE_MODE_DURATION_SEC;

    static_assert((CHIP_CONFIG_ICD_ACTIVE_MODE_DURATION_MS) <= (CHIP_CONFIG_ICD_IDLE_MODE_DURATION_SEC * kMillisecondsPerSecond),
                  "Spec requires the IdleModeDuration be equal or greater to the ActiveModeDuration.");
    uint32_t mActiveInterval_ms = CHIP_CONFIG_ICD_ACTIVE_MODE_DURATION_MS;

    uint16_t mActiveThreshold_ms = CHIP_CONFIG_ICD_ACTIVE_MODE_THRESHOLD_MS;

    uint32_t mICDCounter = 0;

    static_assert((CHIP_CONFIG_ICD_CLIENTS_SUPPORTED_PER_FABRIC) >= 1,
                  "Spec requires the minimum of supported clients per fabric be equal or greater to 1.");
    uint16_t mFabricClientsSupported = CHIP_CONFIG_ICD_CLIENTS_SUPPORTED_PER_FABRIC;

    // SIT ICDs should have a SlowPollingThreshold shorter than or equal to 15s (spec 9.16.1.5)
    static constexpr System::Clock::Milliseconds32 kSITPollingThreshold = System::Clock::Milliseconds32(15000);
    System::Clock::Milliseconds32 mSlowPollingInterval                  = CHIP_DEVICE_CONFIG_ICD_SLOW_POLL_INTERVAL;
    System::Clock::Milliseconds32 mFastPollingInterval                  = CHIP_DEVICE_CONFIG_ICD_FAST_POLL_INTERVAL;

    ICDMode mICDMode = ICDMode::SIT;
};

} // namespace chip
