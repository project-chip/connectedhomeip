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

#ifdef SL_WIFI
#include "wfx_host_events.h"
#endif /* SL_WIFI */

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

#include <crypto/CHIPCryptoPAL.h>
// If building with the EFR32-provided crypto backend, we can use the
// opaque keystore
#if CHIP_CRYPTO_PLATFORM
#include <platform/EFR32/Efr32PsaOperationalKeystore.h>
static chip::DeviceLayer::Internal::Efr32PsaOperationalKeystore gOperationalKeystore;
#endif

#include "EFR32DeviceDataProvider.h"

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

void EFR32MatterConfig::InitOTARequestorHandler(System::Layer * systemLayer, void * appState)
{
    OTAConfig::Init();
}

void EFR32MatterConfig::ConnectivityEventCallback(const ChipDeviceEvent * event, intptr_t arg)
{
    // Initialize OTA only when Thread or WiFi connectivity is established
    if (((event->Type == DeviceEventType::kThreadConnectivityChange) &&
         (event->ThreadConnectivityChange.Result == kConnectivity_Established)) ||
        ((event->Type == DeviceEventType::kInternetConnectivityChange) &&
         (event->InternetConnectivityChange.IPv6 == kConnectivity_Established)))
    {
        EFR32_LOG("Scheduling OTA Requestor initialization")
        chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(OTAConfig::kInitOTARequestorDelaySec),
                                                    InitOTARequestorHandler, nullptr);
    }
}

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
    SetDeviceInstanceInfoProvider(&EFR32::EFR32DeviceDataProvider::GetDeviceDataProvider());
    SetCommissionableDataProvider(&EFR32::EFR32DeviceDataProvider::GetDeviceDataProvider());

    //==============================================
    // Init Matter Stack
    //==============================================
    EFR32_LOG("Init CHIP Stack");
    // Init Chip memory management before the stack
    ReturnErrorOnFailure(chip::Platform::MemoryInit());
    ReturnErrorOnFailure(PlatformMgr().InitChipStack());

    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName(appName);

#if CHIP_ENABLE_OPENTHREAD
    ReturnErrorOnFailure(InitOpenThread());
#endif

    // Stop Matter event handling while setting up resources
    chip::DeviceLayer::PlatformMgr().LockChipStack();

    // Create initParams with SDK example defaults here
    static chip::CommonCaseDeviceServerInitParams initParams;

#if CHIP_CRYPTO_PLATFORM
    // When building with EFR32 crypto, use the opaque key store
    // instead of the default (insecure) one.
    gOperationalKeystore.Init();
    initParams.operationalKeystore = &gOperationalKeystore;
#endif

    // Initialize the remaining (not overridden) providers to the SDK example defaults
    (void) initParams.InitializeStaticResourcesBeforeServerInit();

#if CHIP_ENABLE_OPENTHREAD
    // Set up OpenThread configuration when OpenThread is included
    chip::Inet::EndPointStateOpenThread::OpenThreadEndpointInitParam nativeParams;
    nativeParams.lockCb                = LockOpenThreadTask;
    nativeParams.unlockCb              = UnlockOpenThreadTask;
    nativeParams.openThreadInstancePtr = chip::DeviceLayer::ThreadStackMgrImpl().OTInstance();
    initParams.endpointNativeParams    = static_cast<void *>(&nativeParams);
#endif

    // Init Matter Server and Start Event Loop
    chip::Server::GetInstance().Init(initParams);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    // OTA Requestor initialization will be triggered by the connectivity events
    PlatformMgr().AddEventHandler(ConnectivityEventCallback, reinterpret_cast<intptr_t>(nullptr));

    EFR32_LOG("Starting Platform Manager Event Loop");
    ReturnErrorOnFailure(PlatformMgr().StartEventLoopTask());

#ifdef SL_WIFI
    InitWiFi();
#endif

#ifdef ENABLE_CHIP_SHELL
    chip::startShellTask();
#endif

    return CHIP_NO_ERROR;
}

#ifdef SL_WIFI
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
#endif // SL_WIFI

// ================================================================================
// FreeRTOS Callbacks
// ================================================================================
extern "C" void vApplicationIdleHook(void)
{
    // FreeRTOS Idle callback

    // Check CHIP Config nvm3 and repack flash if necessary.
    Internal::EFR32Config::RepackNvm3Flash();
}
