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

#include <platform/CHIPDeviceLayer.h>

#include <openthread/platform/entropy.h>

#include <AppTask.h>

#include "AppConfig.h"
#include "DataModelHandler.h"
#include "Server.h"
#include "init_board.h"
#include "init_mcu.h"

#if DISPLAY_ENABLED
#include "init_lcd.h"
#endif

#include <FreeRTOS.h>

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
    DemoSessionManager sessions;

    initMcu();
    initBoard();
    efr32RandomInit();

#if DISPLAY_ENABLED
    initLCD();
#endif

#if EFR32_LOG_ENABLED
    if (efr32LogInit() != 0)
    {
        appError(ret);
    }
#endif

    EFR32_LOG("==================================================");
    EFR32_LOG("chip-efr32-lock-example starting");
    EFR32_LOG("==================================================");

    EFR32_LOG("Init CHIP Stack");
    ret = PlatformMgr().InitChipStack();
    if (ret != CHIP_NO_ERROR)
    {
        EFR32_LOG("PlatformMgr().InitChipStack() failed");
        appError(ret);
    }

    EFR32_LOG("Starting Platform Manager Event Loop");
    ret = PlatformMgr().StartEventLoopTask();
    if (ret != CHIP_NO_ERROR)
    {
        EFR32_LOG("PlatformMgr().StartEventLoopTask() failed");
        appError(ret);
    }

    // Init ZCL Data Model
    InitDataModelHandler();
    StartServer(&sessions);

    EFR32_LOG("Init OpenThread Stack");
    ret = ThreadStackMgr().InitThreadStack();
    if (ret != CHIP_NO_ERROR)
    {
        EFR32_LOG("ThreadStackMgr().InitThreadStack(); failed");
        appError(ret);
    }

    EFR32_LOG("Starting App Task");
    ret = GetAppTask().StartAppTask();
    if (ret != CHIP_NO_ERROR)
    {
        EFR32_LOG("GetAppTask().Init() failed");
        appError(ret);
    }

    EFR32_LOG("Starting FreeRTOS scheduler");
    vTaskStartScheduler();

    // Should never get here.
    EFR32_LOG("vTaskStartScheduler() failed");
    appError(ret);
}
