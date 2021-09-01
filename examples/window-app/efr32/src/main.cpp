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

#include <AppConfig.h>
#include <FreeRTOS.h>
#include <WindowApp.h>
#include <app/server/Server.h>
#include <init_efrPlatform.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <mbedtls/threading.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>

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
#include <Rpc.h>
#endif

using namespace ::chip::DeviceLayer;

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
// App Error
//=================================================================================

void appError(CHIP_ERROR err)
{
    // appError(static_cast<int>(err.AsInteger()));
    EFR32_LOG("!!!!!!!!!!!! App Critical Error: %d !!!!!!!!!!!", err);
    portDISABLE_INTERRUPTS();
    while (1)
        ;
}

// ================================================================================
// Main Code
// ================================================================================
int main(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    init_efrPlatform();

#if PW_RPC_ENABLED
    chip::rpc::Init();
#endif

    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);

    EFR32_LOG("==================================================");
    EFR32_LOG("chip-efr32-window-cover-example starting");
    EFR32_LOG("==================================================");

    EFR32_LOG("Init CHIP Stack");

    // Init Chip memory management before the stack
    chip::Platform::MemoryInit();
    chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init();

    err = PlatformMgr().InitChipStack();
    if (err != CHIP_NO_ERROR)
    {
        EFR32_LOG("PlatformMgr().InitChipStack() failed");
        appError(err);
    }
    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName("EFR32_WINDOW");

    EFR32_LOG("Starting Platform Manager Event Loop");
    err = PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        EFR32_LOG("PlatformMgr().StartEventLoopTask() failed");
        appError(err);
    }

#if CHIP_ENABLE_OPENTHREAD
    EFR32_LOG("Initializing OpenThread stack");
    err = ThreadStackMgr().InitThreadStack();
    if (err != CHIP_NO_ERROR)
    {
        EFR32_LOG("ThreadStackMgr().InitThreadStack() failed");
        appError(err);
    }

    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router);
    if (err != CHIP_NO_ERROR)
    {
        EFR32_LOG("ConnectivityMgr().SetThreadDeviceType() failed");
        appError(err);
    }

    EFR32_LOG("Starting OpenThread task");

    // Start OpenThread task
    err = ThreadStackMgrImpl().StartThreadTask();
    if (err != CHIP_NO_ERROR)
    {
        EFR32_LOG("ThreadStackMgr().StartThreadTask() failed");
        appError(err);
    }
#endif // CHIP_ENABLE_OPENTHREAD

    WindowApp & app = WindowApp::Instance();

    EFR32_LOG("Starting App");
    err = app.Init();
    if (err != CHIP_NO_ERROR)
    {
        EFR32_LOG("App Init failed");
        appError(err);
    }

    err = app.Start();
    if (err != CHIP_NO_ERROR)
    {
        EFR32_LOG("App Start failed");
        appError(err);
    }

    app.Finish();
    return err.AsInteger();
}
