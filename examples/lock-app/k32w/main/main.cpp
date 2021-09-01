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

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
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
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
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

/* needed for FreeRtos Heap 4 */
uint8_t __attribute__((section(".heap"))) ucHeap[HEAP_SIZE];

extern "C" void main_task(void const * argument)
{
    /* Call C++ constructors */
    InitFunc * pFunc = &__init_array_start;
    for (; pFunc < &__init_array_end; ++pFunc)
    {
        (*pFunc)();
    }

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
    PWR_Init();

    PWR_vAddRamRetention((uint32_t) &ucHeap[0], sizeof(ucHeap));
    PWR_RegisterLowPowerExitCallback(dm_switch_wakeupCallBack);
    PWR_RegisterLowPowerEnterCallback(dm_switch_preSleepCallBack);

    dm_lp_init();
#endif

    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);

    /* Used for HW initializations */
    otSysInit(0, NULL);

    K32W_LOG("Welcome to NXP ELock Demo App");

    /* Mbedtls Threading support is needed because both
     * Thread and Weave tasks are using it */
    freertos_mbedtls_mutex_init();

    // Init Chip memory management before the stack
    chip::Platform::MemoryInit();

    CHIP_ERROR ret = PlatformMgr().InitChipStack();
    if (ret != CHIP_NO_ERROR)
    {
        K32W_LOG("Error during PlatformMgr().InitWeaveStack()");
        goto exit;
    }

    ret = ThreadStackMgr().InitThreadStack();
    if (ret != CHIP_NO_ERROR)
    {
        K32W_LOG("Error during ThreadStackMgr().InitThreadStack()");
        goto exit;
    }

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
    dualModeStates.threadWarmBootInitTime = THREAD_WARM_BOOT_INIT_DURATION_DEFAULT_VALUE;
    dualModeStates.threadInitialized      = TRUE;
#endif

    ret = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SleepyEndDevice);
    if (ret != CHIP_NO_ERROR)
    {
        goto exit;
    }

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

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
    /* make sure that 15.4 radio is initialized before waking up the Thread task */
    if (dualModeStates.threadInitialized == TRUE)
#endif
    {
        BaseType_t yieldRequired = ThreadStackMgrImpl().SignalThreadActivityPendingFromISR();
        portYIELD_FROM_ISR(yieldRequired);
    }
}

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
uint32_t dm_switch_get15_4InitWakeUpTime(void)
{
    return dualModeStates.threadWarmBootInitTime;
}

extern "C" bleResult_t App_PostCallbackMessage(appCallbackHandler_t handler, appCallbackParam_t param)
{
    AppEvent event;
    event.Type    = AppEvent::kEventType_Lp;
    event.Handler = handler;
    event.param   = param;

    GetAppTask().PostEvent(&event);

    return gBleSuccess_c;
}

static void dm_switch_wakeupCallBack(void)
{
    BOARD_SetClockForWakeup();

    RNG_Init();
    SecLib_Init();

    KBD_PrepareExitLowPower();

    PWR_WakeupReason_t wakeReason = PWR_GetWakeupReason();
    if (wakeReason.Bits.FromBLE_LLTimer == 1)
    {
        SWITCH_DBG_LOG("woken up from LL");
    }
    else if (wakeReason.Bits.FromKeyBoard == 1)
    {
        SWITCH_DBG_LOG("woken up from FromKeyBoard");
    }
    else if (wakeReason.Bits.FromTMR == 1)
    {
        SWITCH_DBG_LOG("woken up from TMR");
    }
    dm_lp_wakeup();
}

static void dm_switch_preSleepCallBack(void)
{
    SWITCH_DBG_LOG("sleeping");

    if (dualModeStates.threadInitialized)
    {
        otPlatRadioDisable(NULL);
        dualModeStates.threadInitialized = FALSE;
    }
    /* Inform the low power dual mode module that we will sleep */
    dm_lp_preSleep();

    /* configure pins for power down mode */
    BOARD_SetPinsForPowerMode();
    /* DeInitialize application support for drivers */
    BOARD_DeInitAdc();
    /* DeInit the necessary clocks */
    BOARD_SetClockForPowerMode();
}

void dm_switch_init15_4AfterWakeUp(void)
{
    uint32_t tick1 = 0;
    uint32_t tick2 = 0;

    if (dualModeStates.threadWarmBootInitTime == THREAD_WARM_BOOT_INIT_DURATION_DEFAULT_VALUE)
    {
        /* Get a 32K tick */
        PWR_Start32kCounter();
        tick1 = PWR_Get32kTimestamp();
    }

    /* Radio must be re-enabled after waking up from sleep.
     * The module is completely disabled in power down mode */
    ThreadExitSleep();

    if (dualModeStates.threadWarmBootInitTime == THREAD_WARM_BOOT_INIT_DURATION_DEFAULT_VALUE)
    {
        tick2                                 = PWR_Get32kTimestamp();
        dualModeStates.threadWarmBootInitTime = ((tick2 - tick1) * 15625u) >> 9;
        /* Add a margin of 1 ms */
        dualModeStates.threadWarmBootInitTime += 1000;
    }
}

static void ThreadExitSleep()
{
    if (!dualModeStates.threadInitialized && ConnectivityMgr().IsThreadEnabled())
    {
        /* Install uMac interrupt handlers */
        OSA_InstallIntHandler(ZIGBEE_MAC_IRQn, vMMAC_IntHandlerBbc);
        OSA_InstallIntHandler(ZIGBEE_MODEM_IRQn, vMMAC_IntHandlerPhy);

        /* Radio must be re-enabled after waking up from sleep.
         * The module is completely disabled in power down mode */
        otPlatRadioEnable(NULL);
        dualModeStates.threadInitialized = TRUE;

        /* wake up the Thread stack and check if any processing needs to be done */
        otTaskletsSignalPending(NULL);
    }
}

extern "C" void App_NotifyWakeup(void)
{
    if ((!dualModeStates.threadInitialized))
    {
        /* Notify the dual mode low power mode */
        (void) App_PostCallbackMessage(dm_lp_processEvent, (void *) e15_4WakeUpEnded);
    }
}

extern "C" void App_AllowDeviceToSleep()
{
    PWR_PreventEnterLowPower(false);
}

extern "C" void App_DisallowDeviceToSleep()
{
    PWR_PreventEnterLowPower(true);
}

static void BOARD_SetClockForWakeup(void)
{
    /* Enables the clock for the I/O controller block. 0: Disable. 1: Enable.: 0x01u */
    CLOCK_EnableClock(kCLOCK_Iocon);
    /* Enables the clock for the GPIO0 module */
    CLOCK_EnableClock(kCLOCK_Gpio0);
}
#endif
