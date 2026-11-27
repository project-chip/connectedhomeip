/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "LowPower.h"
#include "fsl_pm_core.h"
#include "fwk_platform_lowpower.h"
// The "fwk_platform_extflash.h" header file does not export its API for C++
// linkage so we have to force it here until it does
extern "C" {
#include "fwk_platform_extflash.h"
}

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>
#include <platform/nxp/common/PlatformManagerImpl.h>

#if CONFIG_APP_FREERTOS_OS && CONFIG_CHIP_NXP_PLATFORM_RW61X && CONFIG_NXP_USE_LOW_POWER
#include "FreeRTOS.h"
#include "PWR_Interface.h"
#include "task.h"
#include "timers.h"

#endif

#define APP_ERROR_PM_REGISTER_LP_CALLBACK_FAILED CHIP_APPLICATION_ERROR(0x01)

static status_t LowPowerCallback(pm_event_type_t type, uint8_t state, void * data);

static pm_notify_element_t appNotifyElement = {
    .notifyCallback = LowPowerCallback,
    .data           = NULL,
};

static status_t LowPowerCallback(pm_event_type_t type, uint8_t state, void * data)
{
#if CONFIG_CHIP_NXP_PLATFORM_MCXW72
    if (state < PLATFORM_DEEP_SLEEP_STATE)
    {
        /* Nothing to do when entering WFI or Sleep low power state
           NVIC fully functionnal to trigger upcoming interrupts */
        return kStatus_Success;
    }

    // Do nothing if the OTA storage is not enabled
    if (chip::DeviceLayer::PlatformMgrImpl().GetOTAStorageState() != chip::DeviceLayer::PlatformManagerImpl::OTAStorageEnabled)
    {
        return kStatus_Success;
    }

    if (type == kPM_EventEnteringSleep)
    {
        PLATFORM_UninitExternalFlash();
    }
    else
    {
        PLATFORM_ReinitExternalFlash();
    }
#endif

    return kStatus_Success;
}

namespace chip::NXP::App::LowPower {

#if CONFIG_APP_FREERTOS_OS && CONFIG_CHIP_NXP_PLATFORM_RW61X && CONFIG_NXP_USE_LOW_POWER
namespace {

// Adapted from the SDK wireless/coex/src/coex_pm/coex_lpm.c implementation.
constexpr uint32_t kDefaultEnableDurationMs = 10000U;

// Matches the default constraint established by APP_InitServices()
// during HardwareInit(), before the scheduler starts.
PWR_LowpowerMode_t sCurrentMode = PWR_WFI;
bool sTimedModeActive           = false;
TimerHandle_t sLpTimer          = nullptr;

// Callers must hold a FreeRTOS task critical section.
CHIP_ERROR ConfigureNextLowPowerMode(PWR_LowpowerMode_t nextMode, uint32_t timeMs)
{
    if (timeMs > 0U)
    {
        VerifyOrReturnError(sLpTimer != nullptr, CHIP_ERROR_INCORRECT_STATE);

        // A successful period change also starts the timer. Queue the recovery
        // timer before releasing the current constraint.
        VerifyOrReturnError(xTimerChangePeriod(sLpTimer, static_cast<TickType_t>(timeMs / portTICK_PERIOD_MS), 0) == pdPASS,
                            CHIP_ERROR_INTERNAL);
    }

    (void) PWR_ReleaseLowPowerModeConstraint(sCurrentMode);
    (void) PWR_SetLowPowerModeConstraint(nextMode);
    sCurrentMode = nextMode;
    return CHIP_NO_ERROR;
}

void LowPowerTimerCallback(TimerHandle_t timer)
{
    (void) timer;

    (void) ConfigureNextLowPowerMode(PWR_WFI, 0U);
    sTimedModeActive = false;

    ChipLogProgress(AppServer, "Exit low power loop.");
}

CHIP_ERROR CreateLowPowerTimer()
{
    if (!sLpTimer)
    {
        sLpTimer = xTimerCreate("LP timer", static_cast<TickType_t>(kDefaultEnableDurationMs / portTICK_PERIOD_MS), pdFALSE,
                                nullptr, &LowPowerTimerCallback);
        if (!sLpTimer)
        {
            ChipLogError(AppServer, "Failed to create low-power timer");
            return CHIP_ERROR_NO_MEMORY;
        }
    }

    return CHIP_NO_ERROR;
}

} // namespace
#endif

CHIP_ERROR Init()
{
#if CONFIG_APP_FREERTOS_OS && CONFIG_CHIP_NXP_PLATFORM_RW61X && CONFIG_NXP_USE_LOW_POWER
    ReturnErrorOnFailure(CreateLowPowerTimer());
#endif

    status_t status = PM_RegisterNotify(kPM_NotifyGroup2, &appNotifyElement);
    VerifyOrReturnError(status == kStatus_Success, APP_ERROR_PM_REGISTER_LP_CALLBACK_FAILED);

    return CHIP_NO_ERROR;
}

#if CONFIG_APP_FREERTOS_OS && CONFIG_CHIP_NXP_PLATFORM_RW61X && CONFIG_NXP_USE_LOW_POWER
CHIP_ERROR StartTimedMode(uint8_t mode, uint32_t durationMs)
{
    // WFI is restored by the timer; DeepPowerDown powers off the radio firmwares.
    VerifyOrReturnError(mode >= PWR_Sleep && mode <= PWR_PowerDown && durationMs >= portTICK_PERIOD_MS,
                        CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(sLpTimer != nullptr, CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR err = CHIP_NO_ERROR;

    if (sTimedModeActive)
    {
        err = CHIP_ERROR_INCORRECT_STATE;
    }
    else
    {
        // Publish the state before arming the timer. Keep the timer callback
        // from running until the constraint transition is complete.
        sTimedModeActive = true;
        err              = ConfigureNextLowPowerMode(static_cast<PWR_LowpowerMode_t>(mode), durationMs);
        if (err != CHIP_NO_ERROR)
        {
            // The current constraint is unchanged if timer submission fails.
            sTimedModeActive = false;
        }
    }

    if (err == CHIP_ERROR_INTERNAL)
    {
        ChipLogError(AppServer, "Failed to submit low-power timer command");
    }

    return err;
}

void EnableDeepSleep()
{
    (void) ConfigureNextLowPowerMode(PWR_DeepSleep, 0U);
}

void DisableDeepSleep()
{
    (void) ConfigureNextLowPowerMode(PWR_WFI, 0U);
}

#endif

} // namespace chip::NXP::App::LowPower
