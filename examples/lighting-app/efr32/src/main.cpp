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

#include <stdio.h>
#include <stdlib.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include <FreeRTOS.h>
#include <mbedtls/threading.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>

#include <AppTask.h>

#include "AppConfig.h"
#include "init_efrPlatform.h"
#include "sl_simple_button_instances.h"
#include "sl_system_kernel.h"
#include <app/server/Server.h>

#ifdef EFR32_OTA_ENABLED
#include "OTAConfig.h"
#endif // EFR32_OTA_ENABLED

#ifdef HEAP_MONITORING
#include "MemMonitoring.h"
#endif

#if DISPLAY_ENABLED
#include "lcd.h"
#endif

#include <mbedtls/platform.h>
#if CHIP_ENABLE_OPENTHREAD
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

#if defined(RS911X_WIFI) || defined(WF200_WIFI)
#include "wfx_host_events.h"
#endif /* RS911X_WIFI */

#if PW_RPC_ENABLED
#include "Rpc.h"
#endif

#ifdef ENABLE_CHIP_SHELL
#include "matter_shell.h"
#endif

#define BLE_DEV_NAME "SiLabs-Light"
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

    // Check CHIP Config nvm3 and repack flash if necessary.
    Internal::EFR32Config::RepackNvm3Flash();
}

// ================================================================================
// Main Code
// ================================================================================
int main(void)
{
    init_efrPlatform();
    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);

#if PW_RPC_ENABLED
    chip::rpc::Init();
#endif

#ifdef HEAP_MONITORING
    MemMonitoring::startHeapMonitoring();
#endif

    EFR32_LOG("==================================================");
    EFR32_LOG("chip-efr32-lighting-example starting");
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
    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName(BLE_DEV_NAME);
#if CHIP_ENABLE_OPENTHREAD
    EFR32_LOG("Initializing OpenThread stack");
    ret = ThreadStackMgr().InitThreadStack();
    if (ret != CHIP_NO_ERROR)
    {
        EFR32_LOG("ThreadStackMgr().InitThreadStack() failed");
        appError(ret);
    }

#if CHIP_DEVICE_CONFIG_THREAD_FTD
    ret = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router);
#else // CHIP_DEVICE_CONFIG_THREAD_FTD
#if CHIP_DEVICE_CONFIG_ENABLE_SED
    ret = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SleepyEndDevice);
#endif // CHIP_DEVICE_CONFIG_ENABLE_SED
    ret = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice);
#endif // CHIP_DEVICE_CONFIG_THREAD_FTD
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
#ifdef WF200_WIFI
    // Start wfx bus communication task.
    wfx_bus_start();
#ifdef SL_WFX_USE_SECURE_LINK
    wfx_securelink_task_start(); // start securelink key renegotiation task
#endif                           // SL_WFX_USE_SECURE_LINK
#endif                           /* WF200_WIFI */

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
#ifdef RS911X_WIFI
    /*
     * Start up any RSI interface stuff
     * (Not required) - Note that wfx_wifi_start will deal with
     * starting up a rsi task - which will initialize the SPI interface.
     */
#endif
#ifdef EFR32_OTA_ENABLED
    OTAConfig::Init();
#endif // EFR32_OTA_ENABLED

    EFR32_LOG("Starting App Task");
    ret = GetAppTask().StartAppTask();
    if (ret != CHIP_NO_ERROR)
    {
        EFR32_LOG("GetAppTask().Init() failed");
        appError(ret);
    }

#ifdef ENABLE_CHIP_SHELL
    chip::startShellTask();
#endif

    EFR32_LOG("Starting FreeRTOS scheduler");
    sl_system_kernel_start();

    chip::Platform::MemoryShutdown();

    // Should never get here.
    EFR32_LOG("vTaskStartScheduler() failed");
    appError(ret);
}

void sl_button_on_change(const sl_button_t * handle)
{
    GetAppTask().ButtonEventHandler(handle, sl_button_get_state(handle));
}
