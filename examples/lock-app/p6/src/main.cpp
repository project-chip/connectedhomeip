/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include "init_p6Platform.h"
#include <app/server/Server.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;

volatile int apperror_cnt;
// ================================================================================
// App Error
//=================================================================================
void appError(int err)
{
    P6_LOG("!!!!!!!!!!!! App Critical Error: %d !!!!!!!!!!!", err);
    portDISABLE_INTERRUPTS();
    while (1)
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

// ================================================================================
// Main Code
// ================================================================================
int main(void)
{
    init_p6Platform();

    P6_LOG("==================================================\r\n");
    P6_LOG("chip-p6-lock-example starting\r\n");
    P6_LOG("==================================================\r\n");

    // Init Chip memory management before the stack
    chip::Platform::MemoryInit();
    // chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init();

    CHIP_ERROR ret = PlatformMgr().InitChipStack();
    if (ret != CHIP_NO_ERROR)
    {
        P6_LOG("PlatformMgr().InitChipStack() failed");
        appError(ret);
    }
    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName("P6_LOCK");
    P6_LOG("Starting Platform Manager Event Loop");
    ret = PlatformMgr().StartEventLoopTask();
    if (ret != CHIP_NO_ERROR)
    {
        P6_LOG("PlatformMgr().StartEventLoopTask() failed");
        appError(ret);
    }
    ret = GetAppTask().StartAppTask();
    if (ret != CHIP_NO_ERROR)
    {
        P6_LOG("GetAppTask().Init() failed");
        appError(ret);
    }
    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();

    chip::Platform::MemoryShutdown();
    PlatformMgr().StopEventLoopTask();
    PlatformMgr().Shutdown();

    // Should never get here.
    P6_LOG("vTaskStartScheduler() failed");
    appError(ret);
}
