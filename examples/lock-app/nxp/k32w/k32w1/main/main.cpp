/*
 *
 *    Copyright (c) 2020 Google LLC.
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

// ================================================================================
// Main Code
// ================================================================================

#include "openthread/platform/logging.h"
#include <mbedtls/platform.h>
#include <openthread-system.h>
#include <openthread/cli.h>
#include <openthread/error.h>

#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ThreadStackManager.h>

#include "FreeRtosHooks.h"
#include "app_config.h"

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Logging;

#include <AppTask.h>

#if defined(chip_with_low_power) && (chip_with_low_power == 1)
#include "Keyboard.h"
#include "PWR_Configuration.h"
#include "PWR_Interface.h"
#include "RNG_Interface.h"
#include "app_dual_mode_low_power.h"
#include "app_dual_mode_switch.h"
#include "radio.h"
#endif

typedef void (*InitFunc)(void);
extern InitFunc __init_array_start;
extern InitFunc __init_array_end;

/* low power requirements */
#if defined(chip_with_low_power) && (chip_with_low_power == 1)
extern "C" void vMMAC_IntHandlerBbc();
extern "C" void vMMAC_IntHandlerPhy();
extern "C" void BOARD_SetClockForPowerMode(void);

static void dm_switch_wakeupCallBack(void);
static void dm_switch_preSleepCallBack(void);
static void ThreadExitSleep();
static void BOARD_SetClockForWakeup(void);

typedef struct
{
    bool_t bleAppRunning;
    bool_t bleAppStopInprogress;
    bool_t threadInitialized;
    uint32_t threadWarmBootInitTime;
} sDualModeAppStates;

typedef void * appCallbackParam_t;
typedef void (*appCallbackHandler_t)(appCallbackParam_t param);

static sDualModeAppStates dualModeStates;

#define THREAD_WARM_BOOT_INIT_DURATION_DEFAULT_VALUE 4000
#endif

extern "C" void main_task(void const * argument)
{
    CHIP_ERROR ret = CHIP_NO_ERROR;

    /* Call C++ constructors */
    InitFunc * pFunc = &__init_array_start;
    for (; pFunc < &__init_array_end; ++pFunc)
    {
        (*pFunc)();
    }

#if defined(chip_with_low_power) && (chip_with_low_power == 1)
#endif

    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);

    ret = PlatformMgrImpl().InitBoardFwk();
    if (ret != CHIP_NO_ERROR)
    {
        return;
    }

    /* Used for HW initializations */
    otSysInit(0, NULL);

    K32W_LOG("Welcome to NXP ELock Demo App");

    /* Mbedtls Threading support is needed because both
     * Thread and Weave tasks are using it */
    freertos_mbedtls_mutex_init();

    // Init Chip memory management before the stack
    chip::Platform::MemoryInit();

    ret = PlatformMgr().InitChipStack();
    if (ret != CHIP_NO_ERROR)
    {
        K32W_LOG("Error during PlatformMgr().InitChipStack()");
        goto exit;
    }

    ret = ThreadStackMgr().InitThreadStack();
    if (ret != CHIP_NO_ERROR)
    {
        K32W_LOG("Error during ThreadStackMgr().InitThreadStack()");
        goto exit;
    }

    ret = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice);
    if (ret != CHIP_NO_ERROR)
    {
        goto exit;
    }
#if 0
    // Configure the Thread polling behavior for the device.
    {
        ConnectivityManager::ThreadPollingConfig pollingConfig;
        pollingConfig.Clear();
        pollingConfig.ActivePollingIntervalMS   = THREAD_ACTIVE_POLLING_INTERVAL_MS;
        pollingConfig.InactivePollingIntervalMS = THREAD_INACTIVE_POLLING_INTERVAL_MS;

        ret = ConnectivityMgr().SetThreadPollingConfig(pollingConfig);
        if (ret != CHIP_NO_ERROR)
        {
            K32W_LOG("Error during ConnectivityMgr().SetThreadPollingConfig(pollingConfig)");
            goto exit;
        }
    }
#endif
    ret = PlatformMgr().StartEventLoopTask();
    if (ret != CHIP_NO_ERROR)
    {
        K32W_LOG("Error during PlatformMgr().StartEventLoopTask();");
        goto exit;
    }

    // Start OpenThread task
    ret = ThreadStackMgrImpl().StartThreadTask();
    if (ret != CHIP_NO_ERROR)
    {
        K32W_LOG("Error during ThreadStackMgrImpl().StartThreadTask()");
        goto exit;
    }

    ret = GetAppTask().StartAppTask();
    if (ret != CHIP_NO_ERROR)
    {
        K32W_LOG("Error during GetAppTask().StartAppTask()");
        goto exit;
    }

    GetAppTask().AppTaskMain(NULL);

exit:
    return;
}

/**
 * Glue function called directly by the OpenThread stack
 * when system event processing work is pending.
 */
extern "C" void otSysEventSignalPending(void)
{

#if defined(chip_with_low_power) && (chip_with_low_power == 1)
    /* make sure that 15.4 radio is initialized before waking up the Thread task */
    if (dualModeStates.threadInitialized == TRUE)
#endif
    {
        BaseType_t yieldRequired = ThreadStackMgrImpl().SignalThreadActivityPendingFromISR();
        portYIELD_FROM_ISR(yieldRequired);
    }
}
