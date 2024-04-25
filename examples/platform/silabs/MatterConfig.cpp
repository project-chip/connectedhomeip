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
#include "BaseApplication.h"
#include "OTAConfig.h"
#include <MatterConfig.h>
#include <cmsis_os2.h>

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

#ifdef SLI_SI91X_MCU_INTERFACE
#include "wfx_rsi.h"
#endif /* SLI_SI91X_MCU_INTERFACE */

#include <crypto/CHIPCryptoPAL.h>
// If building with the EFR32-provided crypto backend, we can use the
// opaque keystore
#if CHIP_CRYPTO_PLATFORM && !(defined(SLI_SI91X_MCU_INTERFACE))
#include <platform/silabs/efr32/Efr32PsaOperationalKeystore.h>
static chip::DeviceLayer::Internal::Efr32PsaOperationalKeystore gOperationalKeystore;
#endif

#include "SilabsDeviceDataProvider.h"
#include <app/InteractionModelEngine.h>
#include <app/TimerDelegates.h>

#ifdef SL_MATTER_TEST_EVENT_TRIGGER_ENABLED
#include "SilabsTestEventTriggerDelegate.h" // nogncheck
#endif

#if CHIP_CONFIG_SYNCHRONOUS_REPORTS_ENABLED
#include <app/reporting/SynchronizedReportSchedulerImpl.h>
#else
#include <app/reporting/ReportSchedulerImpl.h>
#endif

#include <lib/support/BytesToHex.h>

#ifdef PERFORMANCE_TEST_ENABLED
#include <performance_test_commands.h>
#endif

#include <AppTask.h>

#include <DeviceInfoProviderImpl.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <examples/platform/silabs/SilabsDeviceAttestationCreds.h>

#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

/**********************************************************
 * Defines
 *********************************************************/

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Credentials::Silabs;
using namespace chip::DeviceLayer::Silabs;

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
#if CHIP_DEVICE_CONFIG_THREAD_SSED
    ReturnErrorOnFailure(ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SynchronizedSleepyEndDevice));
#else
    ReturnErrorOnFailure(ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SleepyEndDevice));
#endif
#else  // CHIP_CONFIG_ENABLE_ICD_SERVER
    ReturnErrorOnFailure(ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice));
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
#endif // CHIP_DEVICE_CONFIG_THREAD_FTD

    SILABS_LOG("Starting OpenThread task");
    return ThreadStackMgrImpl().StartThreadTask();
}
#endif // CHIP_ENABLE_OPENTHREAD

namespace {

constexpr uint32_t kMainTaskStackSize = (1024 * 5);
// Task is dynamically allocated with max priority. This task gets deleted once the inits are completed.
constexpr osThreadAttr_t kMainTaskAttr = { .name       = "main",
                                           .attr_bits  = osThreadDetached,
                                           .cb_mem     = NULL,
                                           .cb_size    = 0U,
                                           .stack_mem  = NULL,
                                           .stack_size = kMainTaskStackSize,
                                           .priority   = osPriorityRealtime7 };
osThreadId_t sMainTaskHandle;
static chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

void ApplicationStart(void * unused)
{
    CHIP_ERROR err = SilabsMatterConfig::InitMatter(BLE_DEV_NAME);
    if (err != CHIP_NO_ERROR)
        appError(err);

    gExampleDeviceInfoProvider.SetStorageDelegate(&chip::Server::GetInstance().GetPersistentStorage());
    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Credentials::Silabs::GetSilabsDacProvider());
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    SILABS_LOG("Starting App Task");
    err = AppTask::GetAppTask().StartAppTask();
    if (err != CHIP_NO_ERROR)
        appError(err);

    VerifyOrDie(osThreadTerminate(sMainTaskHandle) == osOK); // Deleting the main task should never fail.
    sMainTaskHandle = nullptr;
}
} // namespace

void SilabsMatterConfig::AppInit()
{
    GetPlatform().Init();

    sMainTaskHandle = osThreadNew(ApplicationStart, nullptr, &kMainTaskAttr);
    SILABS_LOG("Starting scheduler");
    VerifyOrDie(sMainTaskHandle); // We can't proceed if the Main Task creation failed.
    GetPlatform().StartScheduler();

    // Should never get here.
    chip::Platform::MemoryShutdown();
    SILABS_LOG("Start Scheduler Failed");
    appError(CHIP_ERROR_INTERNAL);
}

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
    ReturnErrorOnFailure(InitWiFi());
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
    // TODO: replace with our own init param to avoid double allocation in examples
    static chip::CommonCaseDeviceServerInitParams initParams;

    // Report scheduler and timer delegate instance
    static chip::app::DefaultTimerDelegate sTimerDelegate;
#if CHIP_CONFIG_SYNCHRONOUS_REPORTS_ENABLED
    static chip::app::reporting::SynchronizedReportSchedulerImpl sReportScheduler(&sTimerDelegate);
#else
    static chip::app::reporting::ReportSchedulerImpl sReportScheduler(&sTimerDelegate);
#endif

    initParams.reportScheduler = &sReportScheduler;

#ifdef SL_MATTER_TEST_EVENT_TRIGGER_ENABLED
    static SilabsTestEventTriggerDelegate sTestEventTriggerDelegate;
    initParams.testEventTriggerDelegate = &sTestEventTriggerDelegate;
#endif // SL_MATTER_TEST_EVENT_TRIGGER_ENABLED

#if CHIP_CRYPTO_PLATFORM && !(defined(SLI_SI91X_MCU_INTERFACE))
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

#if CHIP_CONFIG_ENABLE_ICD_SERVER && SLI_SI917
    initParams.appDelegate = &BaseApplication::sAppDelegate;
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER && SLI_SI917
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
CHIP_ERROR SilabsMatterConfig::InitWiFi(void)
{
#ifdef WF200_WIFI
    // Start wfx bus communication task.
    wfx_bus_start();
#ifdef SL_WFX_USE_SECURE_LINK
    wfx_securelink_task_start(); // start securelink key renegotiation task
#endif                           // SL_WFX_USE_SECURE_LINK
#endif                           /* WF200_WIFI */

#ifdef SLI_SI91X_MCU_INTERFACE
    sl_status_t status;
    if ((status = wfx_wifi_rsi_init()) != SL_STATUS_OK)
    {
        SILABS_LOG("wfx_wifi_rsi_init failed with status: %x", status);
        ReturnErrorOnFailure((CHIP_ERROR) status);
    }
#endif // SLI_SI91X_MCU_INTERFACE

    return CHIP_NO_ERROR;
}
#endif // SL_WIFI

// ================================================================================
// FreeRTOS Callbacks
// ================================================================================
extern "C" void vApplicationIdleHook(void)
{
#if SLI_SI91X_MCU_INTERFACE && CHIP_CONFIG_ENABLE_ICD_SERVER
    sl_wfx_host_si91x_sleep_wakeup();
#endif
}
