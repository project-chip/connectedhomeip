/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "AppConfig.h"
#include "init_asrPlatform.h"
#include <AppTask.h>
#include <FreeRTOS.h>
#include <app/server/Server.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;
// ================================================================================
// App Error
//=================================================================================
void appError(int err)
{
    ASR_LOG("!!!!!!!!!!!! App Critical Error: %d !!!!!!!!!!!", err);
    lega_rtos_declare_critical();
    lega_rtos_enter_critical();
    while (1)
        ;
}

void appError(CHIP_ERROR error)
{
    appError(static_cast<int>(error.AsInteger()));
}
// ================================================================================
// Main Code
// ================================================================================
int main(void)
{
    init_asrPlatform();

    CHIP_ERROR ret = GetAppTask().StartAppTask();
    if (ret != CHIP_NO_ERROR)
    {
        ASR_LOG("GetAppTask().Init() failed");
        appError(ret);
    }
    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();

    chip::Platform::MemoryShutdown();
    PlatformMgr().StopEventLoopTask();
    PlatformMgr().Shutdown();

    // Should never get here.
    ASR_LOG("vTaskStartScheduler() failed");
    appError(ret);
}
