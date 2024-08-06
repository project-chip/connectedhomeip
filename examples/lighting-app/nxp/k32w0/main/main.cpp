/*
 *
 *    Copyright (c) 2021 Google LLC.
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

#if PDM_SAVE_IDLE
#include <openthread/platform/settings.h>
#endif

#ifdef K32WMCM_APP_BUILD
#define string string_shadow
#include "AppApi.h"
#include "MMAC.h"
#include "mac_sap.h"
#undef string
#endif

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Logging;

typedef void (*InitFunc)(void);
extern InitFunc __init_array_start;
extern InitFunc __init_array_end;

/* needed for FreeRtos Heap 4 */
uint8_t __attribute__((section(".heap"))) ucHeap[HEAP_SIZE];

extern "C" void sched_enable();

#define NORMAL_PWR_LIMIT 10 /* dBm */

#ifdef K32WMCM_APP_BUILD
/* Must be called before zps_eAplAfInit() */
void APP_SetHighTxPowerMode();

/* Must be called after zps_eAplAfInit() */
void APP_SetMaxTxPower();

#undef HIGH_TX_PWR_LIMIT
#define HIGH_TX_PWR_LIMIT 15 /* dBm */
/* High Tx power */
void APP_SetHighTxPowerMode()
{
    if (CHIP_IS_HITXPOWER_CAPABLE())
        vMMAC_SetTxPowerMode(TRUE);
}

void APP_SetMaxTxPower()
{
    if (CHIP_IS_HITXPOWER_CAPABLE())
        eAppApiPlmeSet(PHY_PIB_ATTR_TX_POWER, HIGH_TX_PWR_LIMIT);
}
#endif

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

    K32W_LOG("Welcome to NXP Lighting Demo App");

    /* Mbedtls Threading support is needed because both
     * Thread and Matter tasks are using it */
    freertos_mbedtls_mutex_init();

    // Init Chip memory management before the stack
    chip::Platform::MemoryInit();

#ifdef K32WMCM_APP_BUILD
    APP_SetHighTxPowerMode();
#endif

#if PDM_SAVE_IDLE
    /* OT Settings needs to be initialized
     * early as XCVR is making use of it */
    otPlatSettingsInit(NULL, NULL, 0);
#endif

    err = PlatformMgr().InitChipStack();
    if (err != CHIP_NO_ERROR)
    {
        K32W_LOG("Error during PlatformMgr().InitMatterStack()");
        goto exit;
    }

    err = ThreadStackMgr().InitThreadStack();
    if (err != CHIP_NO_ERROR)
    {
        K32W_LOG("Error during ThreadStackMgr().InitThreadStack()");
        goto exit;
    }

    /* Enable the MAC scheduler after BLEManagerImpl::_Init() and V2MMAC_Enable().
     * This is needed to register properly the active protocols.
     */
    sched_enable();

    err = ConnectivityMgr().SetThreadDeviceType(CONNECTIVITY_MANAGER_THREAD_DEVICE_TYPE);
    if (err != CHIP_NO_ERROR)
    {
        goto exit;
    }

#ifdef K32WMCM_APP_BUILD
    APP_SetMaxTxPower();
    otPlatRadioSetTransmitPower(ThreadStackMgrImpl().OTInstance(), HIGH_TX_PWR_LIMIT);
#else
    otPlatRadioSetTransmitPower(ThreadStackMgrImpl().OTInstance(), NORMAL_PWR_LIMIT);
#endif

    // Start OpenThread task
    err = ThreadStackMgrImpl().StartThreadTask();
    if (err != CHIP_NO_ERROR)
    {
        K32W_LOG("Error during ThreadStackMgrImpl().StartThreadTask()");
        goto exit;
    }

    err = GetAppTask().StartAppTask();
    if (err != CHIP_NO_ERROR)
    {
        K32W_LOG("Error during GetAppTask().StartAppTask()");
        goto exit;
    }

    err = PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        K32W_LOG("Error during PlatformMgr().StartEventLoopTask();");
        goto exit;
    }

    GetAppTask().AppTaskMain(NULL);

exit:
    return;
}

extern "C" void otSysEventSignalPending(void)
{
    {
        BaseType_t yieldRequired = ThreadStackMgrImpl().SignalThreadActivityPendingFromISR();
        portYIELD_FROM_ISR(yieldRequired);
    }
}
