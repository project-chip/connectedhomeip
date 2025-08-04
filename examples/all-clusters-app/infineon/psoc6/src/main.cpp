/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    Copyright 2021, Cypress Semiconductor Corporation (an Infineon company)
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

#include <stdio.h>
#include <stdlib.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include <FreeRTOS.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>

#include <AppTask.h>

#include "AppConfig.h"
#include "cyhal_wdt.h"
#include "init_psoc6Platform.h"
#include <app/server/Server.h>

#ifdef HEAP_MONITORING
#include "MemMonitoring.h"
#endif
#define MAIN_TASK_STACK_SIZE (4096)
#define MAIN_TASK_PRIORITY 2

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;

volatile int apperror_cnt;
static void main_task(void * pvParameters);

// ================================================================================
// App Error
//=================================================================================
void appError(int err)
{
    PSOC6_LOG("!!!!!!!!!!!! App Critical Error: %d !!!!!!!!!!!", err);
    portDISABLE_INTERRUPTS();
    while (true)
        ;
}

void appError(CHIP_ERROR error)
{
    appError(static_cast<int>(error.AsInteger()));
}

// ================================================================================
// FreeRTOS Callbacks
// ================================================================================
extern "C" void vApplicationIdleHook(void)
{
    // FreeRTOS Idle callback
}

extern "C" void vApplicationDaemonTaskStartupHook()
{
    // Init Chip memory management before the stack
    chip::Platform::MemoryInit();

    /* Create the Main task. */
    xTaskCreate(main_task, "Main task", MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, NULL);
}

static void main_task(void * pvParameters)
{
    CHIP_ERROR ret = chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init();
    if (ret != CHIP_NO_ERROR)
    {
        PSOC6_LOG("PersistedStorage::KeyValueStoreMgrImpl().Init() failed");
        appError(ret);
    }

    ret = PlatformMgr().InitChipStack();
    if (ret != CHIP_NO_ERROR)
    {
        PSOC6_LOG("PlatformMgr().InitChipStack() failed");
        appError(ret);
    }

    ret = chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName("PSOC6_CLUSTERS");
    if (ret != CHIP_NO_ERROR)
    {
        PSOC6_LOG("ConnectivityMgr().SetBLEDeviceName() failed");
        appError(ret);
    }

    PSOC6_LOG("Starting Platform Manager Event Loop");
    ret = PlatformMgr().StartEventLoopTask();
    if (ret != CHIP_NO_ERROR)
    {
        PSOC6_LOG("PlatformMgr().StartEventLoopTask() failed");
        appError(ret);
    }
    ret = GetAppTask().StartAppTask();
    if (ret != CHIP_NO_ERROR)
    {
        PSOC6_LOG("GetAppTask().Init() failed");
        appError(ret);
    }

    /* Delete task */
    vTaskDelete(NULL);
}

// ================================================================================
// Main Code
// ================================================================================
int main(void)
{
    init_psoc6Platform();
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    // Clear watchdog timer (started by bootloader) so that it doesn't trigger a reset
    cyhal_wdt_t wdt_obj;
    cyhal_wdt_init(&wdt_obj, cyhal_wdt_get_max_timeout_ms());
    cyhal_wdt_free(&wdt_obj);
#endif
#ifdef HEAP_MONITORING
    MemMonitoring::startHeapMonitoring();
#endif

    PSOC6_LOG("==================================================\r\n");
    PSOC6_LOG("chip-psoc6-all-clusters-example starting Version %d\r\n", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION);
    PSOC6_LOG("==================================================\r\n");

    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();

    chip::Platform::MemoryShutdown();
    PlatformMgr().StopEventLoopTask();
    PlatformMgr().Shutdown();

    // Should never get here.
    PSOC6_LOG("vTaskStartScheduler() failed");
}
