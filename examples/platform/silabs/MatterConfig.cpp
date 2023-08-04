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
#include <MatterConfig.h>

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
#include <platform/silabs/efr32/Efr32PsaOperationalKeystore.h>
static chip::DeviceLayer::Internal::Efr32PsaOperationalKeystore gOperationalKeystore;
#endif

#include "SilabsDeviceDataProvider.h"
#include "SilabsTestEventTriggerDelegate.h"
#include <app/InteractionModelEngine.h>
#include <lib/support/BytesToHex.h>

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
// SilabsMatterConfig Methods
// ================================================================================

CHIP_ERROR SilabsMatterConfig::InitOpenThread(void)
{
    SILABS_LOG("Initializing OpenThread stack");
    ReturnErrorOnFailure(ThreadStackMgr().InitThreadStack());

#if CHIP_DEVICE_CONFIG_THREAD_FTD
    ReturnErrorOnFailure(ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router));
#else // CHIP_DEVICE_CONFIG_THREAD_FTD
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    ReturnErrorOnFailure(ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SleepyEndDevice));
#else  // CHIP_CONFIG_ENABLE_ICD_SERVER
    ReturnErrorOnFailure(ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice));
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
#endif // CHIP_DEVICE_CONFIG_THREAD_FTD

    SILABS_LOG("Starting OpenThread task");
    return ThreadStackMgrImpl().StartThreadTask();
}
#endif // CHIP_ENABLE_OPENTHREAD

#if SILABS_OTA_ENABLED
void SilabsMatterConfig::InitOTARequestorHandler(System::Layer * systemLayer, void * appState)
{
    OTAConfig::Init();
}
#endif

void SilabsMatterConfig::ConnectivityEventCallback(const ChipDeviceEvent * event, intptr_t arg)
{
    // Initialize OTA only when Thread or WiFi connectivity is established
    if (((event->Type == DeviceEventType::kThreadConnectivityChange) &&
         (event->ThreadConnectivityChange.Result == kConnectivity_Established)) ||
        ((event->Type == DeviceEventType::kInternetConnectivityChange) &&
         (event->InternetConnectivityChange.IPv6 == kConnectivity_Established)))
    {
#if SILABS_OTA_ENABLED
        SILABS_LOG("Scheduling OTA Requestor initialization")
        chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(OTAConfig::kInitOTARequestorDelaySec),
                                                    InitOTARequestorHandler, nullptr);
#endif
    }
}

#if SILABS_TEST_EVENT_TRIGGER_ENABLED
static uint8_t sTestEventTriggerEnableKey[TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
                                                                                          0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
                                                                                          0xcc, 0xdd, 0xee, 0xff };
#endif // SILABS_TEST_EVENT_TRIGGER_ENABLED

CHIP_ERROR SilabsMatterConfig::InitMatter(const char * appName)
{
    CHIP_ERROR err;

    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);

    SILABS_LOG("==================================================");
    SILABS_LOG("%s starting", appName);
    SILABS_LOG("==================================================");

#if PW_RPC_ENABLED
    chip::rpc::Init();
#endif

#ifdef HEAP_MONITORING
    MemMonitoring::startHeapMonitoring();
#endif

    //==============================================
    // Init Matter Stack
    //==============================================
    SILABS_LOG("Init CHIP Stack");
    // Init Chip memory management before the stack
    ReturnErrorOnFailure(chip::Platform::MemoryInit());

// WiFi needs to be initialized after Memory Init for some reason
#ifdef SL_WIFI
    InitWiFi();
#endif

    ReturnErrorOnFailure(PlatformMgr().InitChipStack());

    SetDeviceInstanceInfoProvider(&Silabs::SilabsDeviceDataProvider::GetDeviceDataProvider());
    SetCommissionableDataProvider(&Silabs::SilabsDeviceDataProvider::GetDeviceDataProvider());

    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName(appName);

#if CHIP_ENABLE_OPENTHREAD
    ReturnErrorOnFailure(InitOpenThread());
#endif

    // Stop Matter event handling while setting up resources
    chip::DeviceLayer::PlatformMgr().LockChipStack();

    // Create initParams with SDK example defaults here
    static chip::CommonCaseDeviceServerInitParams initParams;

#if SILABS_TEST_EVENT_TRIGGER_ENABLED
    if (Encoding::HexToBytes(SILABS_TEST_EVENT_TRIGGER_ENABLE_KEY, strlen(SILABS_TEST_EVENT_TRIGGER_ENABLE_KEY),
                             sTestEventTriggerEnableKey,
                             TestEventTriggerDelegate::kEnableKeyLength) != TestEventTriggerDelegate::kEnableKeyLength)
    {
        SILABS_LOG("Failed to convert the EnableKey string to octstr type value");
        memset(sTestEventTriggerEnableKey, 0, sizeof(sTestEventTriggerEnableKey));
    }
    static SilabsTestEventTriggerDelegate testEventTriggerDelegate{ ByteSpan(sTestEventTriggerEnableKey) };
    initParams.testEventTriggerDelegate = &testEventTriggerDelegate;
#endif // SILABS_TEST_EVENT_TRIGGER_ENABLED

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
    err = chip::Server::GetInstance().Init(initParams);

    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    ReturnErrorOnFailure(err);

    // OTA Requestor initialization will be triggered by the connectivity events
    PlatformMgr().AddEventHandler(ConnectivityEventCallback, reinterpret_cast<intptr_t>(nullptr));

    SILABS_LOG("Starting Platform Manager Event Loop");
    ReturnErrorOnFailure(PlatformMgr().StartEventLoopTask());

#ifdef ENABLE_CHIP_SHELL
    chip::startShellTask();
#endif

    return CHIP_NO_ERROR;
}

#ifdef SL_WIFI
void SilabsMatterConfig::InitWiFi(void)
{
#ifdef WF200_WIFI
    // Start wfx bus communication task.
    wfx_bus_start();
#ifdef SL_WFX_USE_SECURE_LINK
    wfx_securelink_task_start(); // start securelink key renegotiation task
#endif                           // SL_WFX_USE_SECURE_LINK
#elif defined(SIWX_917)
    SILABS_LOG("Init RSI 917 Platform");
    if (wfx_rsi_platform() != SL_STATUS_OK)
    {
        SILABS_LOG("RSI init failed");
        return CHIP_ERROR_INTERNAL;
    }
#endif /* WF200_WIFI */
}
#endif // SL_WIFI

// ================================================================================
// FreeRTOS Callbacks
// ================================================================================
extern "C" void vApplicationIdleHook(void)
{
    // FreeRTOS Idle callback
}
