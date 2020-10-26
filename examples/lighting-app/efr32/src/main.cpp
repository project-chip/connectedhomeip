/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include <bsp.h>
#include <stdio.h>
#include <stdlib.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include <FreeRTOS.h>
#include <mbedtls/threading.h>

#include <platform/CHIPDeviceLayer.h>
#include <support/CHIPMem.h>
#include <support/CHIPPlatformMemory.h>

#include <AppTask.h>

#include "AppConfig.h"
#include "DataModelHandler.h"
#include "Server.h"
#include "init_board.h"
#include "init_mcu.h"

#if DISPLAY_ENABLED
#include "lcd.h"
#endif

#if CHIP_ENABLE_OPENTHREAD
#include <mbedtls/platform.h>
#include <openthread/cli.h>
#include <openthread/dataset.h>
#include <openthread/error.h>
#include <openthread/heap.h>
#include <openthread/icmp6.h>
#include <openthread/instance.h>
#include <openthread/link.h>
#include <openthread/platform/openthread-system.h>
#include <openthread/tasklet.h>
#include <openthread/thread.h>
#endif // CHIP_ENABLE_OPENTHREAD

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;

#define UNUSED_PARAMETER(a) (a = a)

volatile int apperror_cnt;
// ================================================================================
// App Error
//=================================================================================
void appError(int err)
{
    EFR32_LOG("!!!!!!!!!!!! App Critical Error: %d !!!!!!!!!!!", err);
    portDISABLE_INTERRUPTS();
    while (1)
        ;
}

// ================================================================================
// FreeRTOS Callbacks
// ================================================================================
extern "C" void vApplicationIdleHook(void)
{
    // FreeRTOS Idle callback

    // Check CHIP Config nvm3 and repack flash if necessary.
    Internal::EFR32Config::RepackNvm3Flash();
}

// ================================================================================
// Main Code
// ================================================================================
int main(void)
{
    int ret = CHIP_ERROR_MAX;

#if CHIP_ENABLE_OPENTHREAD
    initOtSysEFR();
#else
    initMcu();
    initBoard();
    efr32RandomInit();
#if DISPLAY_ENABLED
    initLCD();
#endif
#if EFR32_LOG_ENABLED
    efr32LogInit();
#endif
#endif

    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);

    // Initialize mbedtls threading support on EFR32
    THREADING_setup();

    EFR32_LOG("==================================================");
    EFR32_LOG("chip-efr32-lighting-example starting");
    EFR32_LOG("==================================================");

    EFR32_LOG("Init CHIP Stack");

    // Init Chip memory management before the stack
    chip::Platform::MemoryInit();

    ret = PlatformMgr().InitChipStack();
    if (ret != CHIP_NO_ERROR)
    {
        EFR32_LOG("PlatformMgr().InitChipStack() failed");
        appError(ret);
    }

#if CHIP_ENABLE_OPENTHREAD
    EFR32_LOG("Initializing OpenThread stack");
    ret = ThreadStackMgr().InitThreadStack();
    if (ret != CHIP_NO_ERROR)
    {
        EFR32_LOG("ThreadStackMgr().InitThreadStack() failed");
        appError(ret);
    }

    ret = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router);
    if (ret != CHIP_NO_ERROR)
    {
        EFR32_LOG("ConnectivityMgr().SetThreadDeviceType() failed");
        appError(ret);
    }
#endif // CHIP_ENABLE_OPENTHREAD

    EFR32_LOG("Starting Platform Manager Event Loop");
    ret = PlatformMgr().StartEventLoopTask();
    if (ret != CHIP_NO_ERROR)
    {
        EFR32_LOG("PlatformMgr().StartEventLoopTask() failed");
        appError(ret);
    }

#if CHIP_ENABLE_OPENTHREAD
    EFR32_LOG("Starting OpenThread task");

    // Start OpenThread task
    ret = ThreadStackMgrImpl().StartThreadTask();
    if (ret != CHIP_NO_ERROR)
    {
        EFR32_LOG("ThreadStackMgr().StartThreadTask() failed");
        appError(ret);
    }
#endif // CHIP_ENABLE_OPENTHREAD

    EFR32_LOG("Starting App Task");
    ret = GetAppTask().StartAppTask();
    if (ret != CHIP_NO_ERROR)
    {
        EFR32_LOG("GetAppTask().Init() failed");
        appError(ret);
    }

    EFR32_LOG("Starting FreeRTOS scheduler");
    vTaskStartScheduler();

    chip::Platform::MemoryShutdown();

    // Should never get here.
    EFR32_LOG("vTaskStartScheduler() failed");
    appError(ret);
}
