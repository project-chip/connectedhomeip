/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
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
#include <task.h>

#include <AppTask.h>

#include "AppConfig.h"
#include "init_asrPlatform.h"
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
