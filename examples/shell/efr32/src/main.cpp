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

#include <ChipShellCollection.h>
#include <lib/core/CHIPCore.h>
#include <lib/shell/Engine.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>
#include <support/CHIPMem.h>
#include <support/CHIPPlatformMemory.h>

#include <AppConfig.h>
#include <app/server/Server.h>
#include <init_efrPlatform.h>

#ifdef HEAP_MONITORING
#include "MemMonitoring.h"
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
using namespace ::chip::DeviceLayer;
using chip::Shell::Engine;

#define SHELL_TASK_STACK_SIZE 8192
#define SHELL_TASK_PRIORITY 3
static TaskHandle_t sShellTaskHandle;
#define APP_TASK_STACK_SIZE (1536)
static StackType_t appStack[APP_TASK_STACK_SIZE / sizeof(StackType_t)];
static StaticTask_t appTaskStruct;

// ================================================================================
// Supporting functions
// ================================================================================

void appError(int err)
{
    EFR32_LOG("!!!!!!!!!!!! App Critical Error: %d !!!!!!!!!!!", err);
    portDISABLE_INTERRUPTS();
    while (1)
        ;
}

unsigned int sleep(unsigned int seconds)
{
    const TickType_t xDelay = 1000 * seconds / portTICK_PERIOD_MS;
    vTaskDelay(xDelay);
    return 0;
}

extern "C" void vApplicationIdleHook(void)
{
    // FreeRTOS Idle callback

    // Check CHIP Config nvm3 and repack flash if necessary.
    Internal::EFR32Config::RepackNvm3Flash();
}

static void shell_task(void * args)
{
    Engine::Root().RunMainLoop();
}

// ================================================================================
// Main Code
// ================================================================================

int main(void)
{
    init_efrPlatform();
    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);

#ifdef HEAP_MONITORING
    MemMonitoring::startHeapMonitoring();
#endif

    // Initialize mbedtls threading support on EFR32
    THREADING_setup();

    EFR32_LOG("==================================================");
    EFR32_LOG("chip-efr32-shell-example starting");
    EFR32_LOG("==================================================");

    EFR32_LOG("Init CHIP Stack");

    // Init Chip memory management before the stack
    chip::Platform::MemoryInit();
    chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init();

    CHIP_ERROR ret = PlatformMgr().InitChipStack();
    if (ret != CHIP_NO_ERROR)
    {
        EFR32_LOG("PlatformMgr().InitChipStack() failed");
        appError(ret);
    }
    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName("EFR32_SHELL");
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

    ret = chip::Shell::streamer_init(chip::Shell::streamer_get());
    assert(ret == 0);

    cmd_otcli_init();
    cmd_ping_init();
    cmd_send_init();

    sShellTaskHandle = xTaskCreateStatic(shell_task, APP_TASK_NAME, ArraySize(appStack), NULL, 1, appStack, &appTaskStruct);
    if (!sShellTaskHandle)
    {
        EFR32_LOG("MEMORY ERROR!!!");
    }

    vTaskStartScheduler();
}
