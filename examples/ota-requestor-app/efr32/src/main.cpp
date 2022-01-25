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

#ifdef HEAP_MONITORING
#include "MemMonitoring.h"
#endif

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

#if PW_RPC_ENABLED
#include "Rpc.h"
#endif

#ifdef ENABLE_CHIP_SHELL
#include "matter_shell.h"
#endif

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;

#define UNUSED_PARAMETER(a) (a = a)

volatile int apperror_cnt;

#include "platform/bootloader/api/application_properties.h"

// Header used for building the image GBL file
#define APP_PROPERTIES_VERSION 1
#define APP_PROPERTIES_ID                                                                                                          \
    {                                                                                                                              \
        0                                                                                                                          \
    }

__attribute__((used)) ApplicationProperties_t sl_app_properties = {
    /// @brief Magic value indicating that this is an ApplicationProperties_t
    /// Must equal @ref APPLICATION_PROPERTIES_MAGIC
    .magic = APPLICATION_PROPERTIES_MAGIC,

    /// Version number of this struct
    .structVersion = APPLICATION_PROPERTIES_VERSION,

    /// Type of signature this application is signed with
    .signatureType = APPLICATION_SIGNATURE_NONE,

    /// Location of the signature. Typically a pointer to the end of application
    .signatureLocation = 0,

    /// Information about the application
    .app = {

      /// Bitfield representing type of application
      /// e.g. @ref APPLICATION_TYPE_BLUETOOTH_APP
      .type = APPLICATION_TYPE_THREAD,

      /// Version number for this application
      .version = APP_PROPERTIES_VERSION,

      /// Capabilities of this application
      .capabilities = 0,

      /// Unique ID (e.g. UUID/GUID) for the product this application is built for
      .productId = APP_PROPERTIES_ID,
    },
};

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
    EFR32_LOG("chip-efr32-ota-requestor-example starting");
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
    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName("EFR32_OTA_REQUESTOR");
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
#else
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
