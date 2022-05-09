/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2022 Silabs.
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
#include "OTAConfig.h"
#include <matter_config.h>

#include <FreeRTOS.h>

#include <mbedtls/platform.h>

#if SILABS_WIFI
#include "wfx_host_events.h"
#endif /* RS911X_WIFI */

#if PW_RPC_ENABLED
#include "Rpc.h"
#endif

#ifdef ENABLE_CHIP_SHELL
#include "matter_shell.h"
#endif

#ifdef HEAP_MONITORING
#include "MemMonitoring.h"
#endif

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;

#if CHIP_ENABLE_OPENTHREAD
#include <inet/EndPointStateOpenThread.h>
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

// ================================================================================
// Matter Networking Callbacks
// ================================================================================
void LockOpenThreadTask(void)
{
    chip::DeviceLayer::ThreadStackMgr().LockThreadStack();
}

void UnlockOpenThreadTask(void)
{
    chip::DeviceLayer::ThreadStackMgr().UnlockThreadStack();
}

// ================================================================================
// EFR32MatterConfig Methods
// ================================================================================

CHIP_ERROR EFR32MatterConfig::InitOpenThread(void)
{
    EFR32_LOG("Initializing OpenThread stack");
    ReturnErrorOnFailure(ThreadStackMgr().InitThreadStack());

#if CHIP_DEVICE_CONFIG_THREAD_FTD
    ReturnErrorOnFailure(ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router));
#else // CHIP_DEVICE_CONFIG_THREAD_FTD
#if CHIP_DEVICE_CONFIG_ENABLE_SED
    ReturnErrorOnFailure(ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SleepyEndDevice));
#else  // CHIP_DEVICE_CONFIG_ENABLE_SED
    ReturnErrorOnFailure(ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice));
#endif // CHIP_DEVICE_CONFIG_ENABLE_SED
#endif // CHIP_DEVICE_CONFIG_THREAD_FTD

    EFR32_LOG("Starting OpenThread task");
    return ThreadStackMgrImpl().StartThreadTask();
}
#endif // CHIP_ENABLE_OPENTHREAD

CHIP_ERROR EFR32MatterConfig::InitMatter(const char * appName)
{
    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);

    EFR32_LOG("==================================================");
    EFR32_LOG("%s starting", appName);
    EFR32_LOG("==================================================");

#if PW_RPC_ENABLED
    chip::rpc::Init();
#endif

#ifdef HEAP_MONITORING
    MemMonitoring::startHeapMonitoring();
#endif

    //==============================================
    // Init Matter Stack
    //==============================================
    EFR32_LOG("Init CHIP Stack");
    // Init Chip memory management before the stack
    chip::Platform::MemoryInit();
    ReturnErrorOnFailure(PlatformMgr().InitChipStack());

    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName(appName);

#if CHIP_ENABLE_OPENTHREAD
    ReturnErrorOnFailure(InitOpenThread());
#endif

    // Init Matter Server and Start Event Loop
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
#if CHIP_ENABLE_OPENTHREAD
    chip::Inet::EndPointStateOpenThread::OpenThreadEndpointInitParam nativeParams;
    nativeParams.lockCb                = LockOpenThreadTask;
    nativeParams.unlockCb              = UnlockOpenThreadTask;
    nativeParams.openThreadInstancePtr = chip::DeviceLayer::ThreadStackMgrImpl().OTInstance();
    initParams.endpointNativeParams    = static_cast<void *>(&nativeParams);
#endif
    chip::Server::GetInstance().Init(initParams);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    EFR32_LOG("Starting Platform Manager Event Loop");
    ReturnErrorOnFailure(PlatformMgr().StartEventLoopTask());

#if SILABS_WIFI
    InitWiFi();
#endif
    // Init Matter OTA
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    OTAConfig::Init();
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

#ifdef ENABLE_CHIP_SHELL
    chip::startShellTask();
#endif

    return CHIP_NO_ERROR;
}

#if SILABS_WIFI
void EFR32MatterConfig::InitWiFi(void)
{
#ifdef WF200_WIFI
    // Start wfx bus communication task.
    wfx_bus_start();
#ifdef SL_WFX_USE_SECURE_LINK
    wfx_securelink_task_start(); // start securelink key renegotiation task
#endif                           // SL_WFX_USE_SECURE_LINK
#endif                           /* WF200_WIFI */

#ifdef RS911X_WIFI
    /*
     * Start up any RSI interface stuff
     * (Not required) - Note that wfx_wifi_start will deal with
     * starting up a rsi task - which will initialize the SPI interface.
     */
#endif
}
#endif // SILABS_WIFI

// ================================================================================
// FreeRTOS Callbacks
// ================================================================================
extern "C" void vApplicationIdleHook(void)
{
    // FreeRTOS Idle callback

    // Check CHIP Config nvm3 and repack flash if necessary.
    Internal::EFR32Config::RepackNvm3Flash();
}
