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

#include <mbedtls/platform.h>

#include <AppTask.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ThreadStackManager.h>

#include "FreeRtosHooks.h"
#include "app_config.h"

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Logging;

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
#include "Keyboard.h"
#include "OtaSupport.h"
#include "PWR_Configuration.h"
#include "PWR_Interface.h"
#include "RNG_Interface.h"
#include "app_dual_mode_low_power.h"
#include "app_dual_mode_switch.h"
#include "radio.h"
#endif

#include "MacSched.h"

typedef void (*InitFunc)(void);

extern InitFunc __init_array_start;
extern InitFunc __init_array_end;

extern "C" void sched_enable();

/* low power requirements */
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
extern "C" void setThreadInitialized(bool isInitialized);
extern "C" bool isThreadInitialized();
#endif

/* needed for FreeRtos Heap 4 */
uint8_t __attribute__((section(".heap"))) ucHeap[HEAP_SIZE];

extern "C" void main_task(void const * argument)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    /* Call C++ constructors */
    InitFunc * pFunc = &__init_array_start;
    for (; pFunc < &__init_array_end; ++pFunc)
    {
        (*pFunc)();
    }

    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);
    err = PlatformMgrImpl().InitBoardFwk();
    if (err != CHIP_NO_ERROR)
    {
        return;
    }

    K32W_LOG("Welcome to NXP ELock Demo App");

    /* Mbedtls Threading support is needed because both
     * Thread and Matter tasks are using it */
    freertos_mbedtls_mutex_init();

    // Init Chip memory management before the stack
    chip::Platform::MemoryInit();

    CHIP_ERROR ret = PlatformMgr().InitChipStack();
    if (ret != CHIP_NO_ERROR)
    {
        K32W_LOG("Error during PlatformMgr().InitMatterStack()");
        goto exit;
    }

    ret = ThreadStackMgr().InitThreadStack();
    if (ret != CHIP_NO_ERROR)
    {
        K32W_LOG("Error during ThreadStackMgr().InitThreadStack()");
        goto exit;
    }

    /* Enable the MAC scheduler after BLEManagerImpl::_Init() and V2MMAC_Enable().
     * This is needed to register properly the active protocols.
     */
    sched_enable();

    ret = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SleepyEndDevice);
    if (ret != CHIP_NO_ERROR)
    {
        goto exit;
    }

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
    setThreadInitialized(TRUE);
#endif

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

    ret = PlatformMgr().StartEventLoopTask();
    if (ret != CHIP_NO_ERROR)
    {
        K32W_LOG("Error during PlatformMgr().StartEventLoopTask();");
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
    if (isThreadInitialized())
#endif
    {
        BaseType_t yieldRequired = ThreadStackMgrImpl().SignalThreadActivityPendingFromISR();
        portYIELD_FROM_ISR(yieldRequired);
    }
}
