/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <app/icd/server/ICDConfigurationData.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/silabs/wifi/WifiInterface.h>
#include <platform/silabs/wifi/icd/WifiSleepManager.h>

using namespace chip::DeviceLayer::Silabs;

namespace {

// TODO: Once the platform sleep calls are unified, we can removed this ifdef
#if SLI_SI917 // 917 SoC & NCP

/**
 * @brief Configures the Wi-Fi Chip to go to DTIM based sleep.
 *        Function sets the listen interval to be synced with the DTIM beacon and disables the broadcast filter.
 *
 * @return CHIP_ERROR CHIP_NO_ERROR if the configuration of the Wi-Fi chip was successful; otherwise CHIP_ERROR_INTERNAL
 */
CHIP_ERROR ConfigureDTIMBasedSleep()
{
    ReturnLogErrorOnFailure(WifiInterface::GetInstance().ConfigureBroadcastFilter(false));

    // Allowing the device to go to sleep must be the last actions to avoid configuration failures.
    ReturnLogErrorOnFailure(WifiInterface::GetInstance().ConfigurePowerSave(RSI_SLEEP_MODE_2, ASSOCIATED_POWER_SAVE, 0));

    return CHIP_NO_ERROR;
}

/**
 * @brief Configures the Wi-Fi chip to go Deep Sleep.
 *        Function doesn't change the state of the broadcast filter.
 *
 * @return CHIP_ERROR CHIP_NO_ERROR if the configuration of the Wi-Fi chip was successful; otherwise CHIP_ERROR_INTERNAL
 */
CHIP_ERROR ConfigureDeepSleep()
{
    ReturnLogErrorOnFailure(WifiInterface::GetInstance().ConfigurePowerSave(RSI_SLEEP_MODE_8, DEEP_SLEEP_WITH_RAM_RETENTION, 0));
    return CHIP_NO_ERROR;
}

/**
 * @brief Configures the Wi-Fi chip to go to High Performance.
 *        Function doesn't change the broad cast filter configuration.
 *
 * @return CHIP_ERROR CHIP_NO_ERROR if the configuration of the Wi-Fi chip was successful; otherwise CHIP_ERROR_INTERNAL
 */
CHIP_ERROR ConfigureHighPerformance()
{
    ReturnLogErrorOnFailure(WifiInterface::GetInstance().ConfigurePowerSave(RSI_ACTIVE, HIGH_PERFORMANCE, 0));
    return CHIP_NO_ERROR;
}
#endif // SLI_SI917

} // namespace

namespace chip {
namespace DeviceLayer {
namespace Silabs {

// Initialize the static instance
WifiSleepManager WifiSleepManager::mInstance;

CHIP_ERROR WifiSleepManager::Init()
{
    return VerifyAndTransitionToLowPowerMode(PowerEvent::kGenericEvent);
}

CHIP_ERROR WifiSleepManager::RequestHighPerformance()
{
    VerifyOrReturnError(mHighPerformanceRequestCounter < std::numeric_limits<uint8_t>::max(), CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "High performance request counter overflow"));

    mHighPerformanceRequestCounter++;

    // We don't do the mHighPerformanceRequestCounter check here; the check is in the VerifyAndTransitionToLowPowerMode function
    ReturnErrorOnFailure(VerifyAndTransitionToLowPowerMode(PowerEvent::kGenericEvent));

    return CHIP_NO_ERROR;
}

CHIP_ERROR WifiSleepManager::RemoveHighPerformanceRequest()
{
    VerifyOrReturnError(mHighPerformanceRequestCounter > 0, CHIP_NO_ERROR,
                        ChipLogError(DeviceLayer, "Wi-Fi configuration already in low power mode"));

    mHighPerformanceRequestCounter--;

    // We don't do the mHighPerformanceRequestCounter check here; the check is in the VerifyAndTransitionToLowPowerMode function
    ReturnErrorOnFailure(VerifyAndTransitionToLowPowerMode(PowerEvent::kGenericEvent));

    return CHIP_NO_ERROR;
}

CHIP_ERROR WifiSleepManager::HandlePowerEvent(PowerEvent event)
{
    switch (event)
    {
    case PowerEvent::kCommissioningComplete:
        ChipLogProgress(AppServer, "WifiSleepManager: Handling Commissioning Complete Event");
        mIsCommissioningInProgress = false;

        // TODO: Remove High Performance Req during commissioning when sleep issues are resolved
        WifiSleepManager::GetInstance().RemoveHighPerformanceRequest();
        break;

    case PowerEvent::kConnectivityChange:
    case PowerEvent::kGenericEvent:
        // No additional processing needed for these events at the moment
        break;

    default:
        ChipLogError(AppServer, "Unknown Power Event");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR WifiSleepManager::VerifyAndTransitionToLowPowerMode(PowerEvent event)
{
    ReturnErrorOnFailure(HandlePowerEvent(event));

#if SLI_SI917 // 917 SoC & NCP
    if (mHighPerformanceRequestCounter > 0)
    {
        return ConfigureHighPerformance();
    }

    if (mIsCommissioningInProgress)
    {
        // During commissioning, don't let the device go to sleep
        // This is needed to interrupt the sleep and retry joining the network
        return CHIP_NO_ERROR;
    }

    if (!WifiInterface::GetInstance().IsWifiProvisioned())
    {
        return ConfigureDeepSleep();
    }

    return ConfigureDTIMBasedSleep();

#else
    ReturnErrorOnFailure(WifiInterface::GetInstance().ConfigurePowerSave());
    return CHIP_NO_ERROR;
#endif
}

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
