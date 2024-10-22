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

#include <app/icd/server/ICDServerConfig.h>
#include <lib/core/Optional.h>
#include <lib/support/TimeUtils.h>
#include <platform/CHIPDeviceConfig.h>
#include <protocols/secure_channel/CheckInCounter.h>
#include <system/SystemClock.h>

namespace chip {

namespace app {
// Forward declaration of ICDManager to allow it to be friend with ICDConfigurationData.
class ICDManager;
} // namespace app

namespace Test {
// Forward declaration of ICDConfigurationDataTestAccess tests to allow it to be friend with the ICDConfigurationData.
// Used in unit tests
class ICDConfigurationDataTestAccess;
} // namespace Test

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
    static constexpr uint32_t kICDCounterPersistenceIncrement = 100;

    enum class ICDMode : uint8_t
    {
        SIT, // Short Interval Time ICD
        LIT, // Long Interval Time ICD
    };

    static ICDConfigurationData & GetInstance() { return instance; };

    System::Clock::Seconds32 GetIdleModeDuration() { return mIdleModeDuration; }

    System::Clock::Milliseconds32 GetActiveModeDuration() { return mActiveModeDuration; }

    System::Clock::Milliseconds16 GetActiveModeThreshold() { return mActiveThreshold; }

    System::Clock::Milliseconds32 GetGuaranteedStayActiveDuration() { return kGuaranteedStayActiveDuration; }

    Protocols::SecureChannel::CheckInCounter & GetICDCounter() { return mICDCounter; }

    uint16_t GetClientsSupportedPerFabric() { return mFabricClientsSupported; }

    System::Clock::Milliseconds32 GetSITPollingThreshold() { return kSITPollingThreshold; }

    System::Clock::Milliseconds32 GetFastPollingInterval() { return mFastPollingInterval; }

    System::Clock::Milliseconds16 GetMinLitActiveModeThreshold() { return kMinLitActiveModeThreshold; }

    System::Clock::Seconds32 GetMaximumCheckInBackoff() { return mMaximumCheckInBackOff; }

    /**
     * The returned value will depend on the devices operating mode.
     * If ICDMode == SIT && the configured slow poll interval is superior to the maximum threshold (15s), the function will return
     * the threshold kSITPollingThreshold (<= 15s). If ICDMode == SIT but the configured slow poll interval is equal or inferior to
     * the threshold, the function will the return the configured slow poll interval. If ICDMode == LIT, the function will return
     * the configured slow poll interval.
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
    // value is changed, they can leverage the Observer events the ICDManager generates. See src/app/icd/server/ICDStateObserver.h
    friend class chip::app::ICDManager;

    friend class chip::Test::ICDConfigurationDataTestAccess;

    void SetICDMode(ICDMode mode) { mICDMode = mode; };
    void SetSlowPollingInterval(System::Clock::Milliseconds32 slowPollInterval) { mSlowPollingInterval = slowPollInterval; };
    void SetFastPollingInterval(System::Clock::Milliseconds32 fastPollInterval) { mFastPollingInterval = fastPollInterval; };

    static constexpr System::Clock::Milliseconds16 kMinLitActiveModeThreshold = System::Clock::Milliseconds16(5000);

    /**
     * @brief Change the ActiveModeDuration or the IdleModeDuration value
     *        If only one value is provided, check will be done agaisn't the other already set value.
     *
     * @param[in] activeModeDuration new ActiveModeDuration value
     * @param[in] idleModeDuration new IdleModeDuration value
     *                                The precision of the IdleModeDuration must be seconds.
     * @return CHIP_ERROR CHIP_ERROR_INVALID_ARGUMENT is returned if idleModeDuration_ms is smaller than activeModeDuration_ms
     *                                                is returned if idleModeDuration_ms is greater than 64800000 ms
     *                                                is returned if idleModeDuration_ms is smaller than 1000 ms
     *                                                is returned if no valid values are provided
     *                    CHIP_NO_ERROR is returned if the new intervals were set
     */
    CHIP_ERROR SetModeDurations(Optional<System::Clock::Milliseconds32> activeModeDuration,
                                Optional<System::Clock::Milliseconds32> idleModeDuration);

    static constexpr System::Clock::Seconds32 kMaxIdleModeDuration = System::Clock::Seconds32(18 * kSecondsPerHour);
    static constexpr System::Clock::Seconds32 kMinIdleModeDuration = System::Clock::Seconds32(1);
    // As defined in the spec, the maximum guaranteed duration for the StayActiveDuration is 30s  "Matter Application
    // Clusters: 9.17.7.5.1. PromisedActiveDuration Field"
    static constexpr System::Clock::Milliseconds32 kGuaranteedStayActiveDuration = System::Clock::Milliseconds32(30000);

    static_assert((CHIP_CONFIG_ICD_IDLE_MODE_DURATION_SEC) <= kMaxIdleModeDuration.count(),
                  "Spec requires the IdleModeDuration to be equal or inferior to 64800s.");
    static_assert((CHIP_CONFIG_ICD_IDLE_MODE_DURATION_SEC) >= kMinIdleModeDuration.count(),
                  "Spec requires the IdleModeDuration to be equal or greater to 1s.");
    System::Clock::Seconds32 mIdleModeDuration = System::Clock::Seconds32(CHIP_CONFIG_ICD_IDLE_MODE_DURATION_SEC);

    static_assert(System::Clock::Milliseconds32(CHIP_CONFIG_ICD_ACTIVE_MODE_DURATION_MS) <=
                      System::Clock::Seconds32(CHIP_CONFIG_ICD_IDLE_MODE_DURATION_SEC),
                  "Spec requires the IdleModeDuration be equal or greater to the ActiveModeDuration.");
    System::Clock::Milliseconds32 mActiveModeDuration = System::Clock::Milliseconds32(CHIP_CONFIG_ICD_ACTIVE_MODE_DURATION_MS);

    System::Clock::Milliseconds16 mActiveThreshold = System::Clock::Milliseconds16(CHIP_CONFIG_ICD_ACTIVE_MODE_THRESHOLD_MS);

    Protocols::SecureChannel::CheckInCounter mICDCounter;

    static_assert((CHIP_CONFIG_ICD_CLIENTS_SUPPORTED_PER_FABRIC) >= 1,
                  "Spec requires the minimum of supported clients per fabric be equal or greater to 1.");
    uint16_t mFabricClientsSupported = CHIP_CONFIG_ICD_CLIENTS_SUPPORTED_PER_FABRIC;

    static_assert((CHIP_CONFIG_ICD_MAXIMUM_CHECK_IN_BACKOFF_SEC) <= kMaxIdleModeDuration.count(),
                  "Spec requires the MaximumCheckInBackOff to be equal or inferior to 64800s");
    static_assert((CHIP_CONFIG_ICD_IDLE_MODE_DURATION_SEC) <= (CHIP_CONFIG_ICD_MAXIMUM_CHECK_IN_BACKOFF_SEC),
                  "Spec requires the MaximumCheckInBackOff to be equal or superior to the IdleModeDuration");
    System::Clock::Seconds32 mMaximumCheckInBackOff = System::Clock::Seconds32(CHIP_CONFIG_ICD_MAXIMUM_CHECK_IN_BACKOFF_SEC);

    // SIT ICDs SHALL have a SlowPollingThreshold shorter than or equal to 15s (spec 9.16.1.5)
    static constexpr System::Clock::Milliseconds32 kSitIcdSlowPollMaximum = System::Clock::Milliseconds32(15000);
    static_assert((CHIP_DEVICE_CONFIG_ICD_SIT_SLOW_POLL_LIMIT).count() <= kSitIcdSlowPollMaximum.count(),
                  "Spec requires the maximum slow poll interval for the SIT device to be smaller or equal than 15 s.");
    static constexpr System::Clock::Milliseconds32 kSITPollingThreshold = CHIP_DEVICE_CONFIG_ICD_SIT_SLOW_POLL_LIMIT;

#if CHIP_CONFIG_ENABLE_ICD_LIT == 0
    static_assert((CHIP_DEVICE_CONFIG_ICD_SLOW_POLL_INTERVAL <= kSitIcdSlowPollMaximum),
                  "LIT support is required for slow polling intervals superior to 15 seconds");
#endif
    System::Clock::Milliseconds32 mSlowPollingInterval = CHIP_DEVICE_CONFIG_ICD_SLOW_POLL_INTERVAL;
    System::Clock::Milliseconds32 mFastPollingInterval = CHIP_DEVICE_CONFIG_ICD_FAST_POLL_INTERVAL;

    ICDMode mICDMode = ICDMode::SIT;
};

} // namespace chip
