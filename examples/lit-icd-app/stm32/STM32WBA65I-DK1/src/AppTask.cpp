/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/*STM32 includes*/
#include "AppTask.h"
#include "AppEvent.h"
#include "IdentifierEffect.h"
#include "app_common.h"
#include "app_nvm.h"
#include "app_thread.h"
#include "cmsis_os2.h"
#include "dbg_trace.h"
#include "main.h"
#include "stm32_lpm.h"
#if (OTA_SUPPORT == 1)
#include "ota.h"
#endif /* (OTA_SUPPORT == 1) */

#if HIGHWATERMARK
#include "mbedtls/memory_buffer_alloc.h"
#endif

/*Matter includes*/
#include <DeviceInfoProviderImpl.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <data-model-providers/codegen/Instance.h>
#include <inet/EndPointStateOpenThread.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>
#if CHIP_CONFIG_SYNCHRONOUS_REPORTS_ENABLED
#include <app/reporting/SynchronizedReportSchedulerImpl.h>
#else
#include <app/reporting/ReportSchedulerImpl.h>
#endif
#if CHIP_ENABLE_OPENTHREAD
#include <platform/OpenThread/GenericNetworkCommissioningThreadDriver.h>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>
#endif

#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/software-diagnostics-server/SoftwareDiagnosticsTestEventTriggerHandler.h>
#if (OTA_SUPPORT == 1)
#include <app/clusters/ota-requestor/OTATestEventTriggerHandler.h>
#endif /* (OTA_SUPPORT == 1) */
// #if (CHIP_CONFIG_ENABLE_ICD_SERVER == 1)
// #include <app/icd/server/ICDManager.h>
// #endif /* (CHIP_CONFIG_ENABLE_ICD_SERVER == 1) */

#include "app_bsp.h"
#include "app_conf.h"
#include "stm32wba65i_discovery.h"

using namespace ::chip;
using namespace ::chip::app;
using namespace chip::TLV;
using namespace chip::Credentials;
using namespace chip::DeviceLayer;
using namespace ::chip::Platform;
using namespace ::chip::Credentials;

using namespace ::chip::app::Clusters;
using chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr;

AppTask AppTask::sAppTask;
chip::DeviceLayer::FactoryDataProvider mFactoryDataProvider;

#define APP_EVENT_QUEUE_SIZE 10
#define NVM_TIMEOUT 1000 // timer to handle PB to save data in nvm or do a factory reset

static QueueHandle_t sAppEventQueue;

#if CHIP_ENABLE_OPENTHREAD
Clusters::NetworkCommissioning::InstanceAndDriver<DeviceLayer::NetworkCommissioning::GenericThreadDriver>
    sThreadNetworkDriver(0 /*endpointId*/);
#endif // CHIP_ENABLE_OPENTHREAD

const osThreadAttr_t AppTask_attr = {
    .name       = APPTASK_NAME,
    .attr_bits  = APP_ATTR_BITS,
    .cb_mem     = APP_CB_MEM,
    .cb_size    = APP_CB_SIZE,
    .stack_mem  = APP_STACK_MEM,
    .stack_size = APP_STACK_SIZE,
    .priority   = APP_PRIORITY,
};

static bool sIsThreadProvisioned          = false;
static bool sIsThreadEnabled              = false;
static bool sHaveBLEConnections           = false;
static bool sFabricNeedSaved              = false;
static bool sFailCommissioning            = false;
static bool sHaveFabric                   = false;
static bool sGenericSwitchLongPressActive = false;

chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

namespace {
constexpr chip::EndpointId kGenericSwitchEndpoint = 1;
} // namespace

CHIP_ERROR AppTask::StartAppTask()
{
    sAppEventQueue = xQueueCreate(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent));
    if (sAppEventQueue == NULL)
    {
        LOG_ERROR_APP("Failed to allocate app event queue\n");
        return CHIP_ERROR_NO_MEMORY;
    }

    /* Start App task. */
    osThreadNew(AppTaskMain, NULL, &AppTask_attr);

    return CHIP_NO_ERROR;
}

void LockOpenThreadTask(void)
{
    APP_THREAD_LockThreadStack();
}

void UnlockOpenThreadTask(void)
{
    APP_THREAD_UnLockThreadStack();
}

CHIP_ERROR AppTask::Init()
{

    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(NotSpecified, "Current Software Version: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);

    /* Setup button handler */
    APPE_PushButtonSetReceiveCb(ButtonEventHandler);

    err = ThreadStackMgr().InitThreadStack();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERROR_APP("ThreadStackMgr().InitThreadStack() failed\n");
        return err;
    }

    err = PlatformMgr().AddEventHandler(MatterEventHandler, 0);
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERROR_APP("PlatformMgr().AddEventHandler() failed\n");
        return err;
    }

    err = IdentifierEffect_Init();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERROR_APP("IdentifierEffect_Init() failed\n");
        return err;
    }

#if (CHIP_DEVICE_CONFIG_ENABLE_SED == 1)
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SleepyEndDevice);
#else
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_FullEndDevice);
#endif
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERROR_APP("ConnectivityMgr().SetThreadDeviceType() failed\n");
        return err;
    }

    err = sThreadNetworkDriver.Init();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERROR_APP("sThreadNetworkDriver.Init() failed\n");
        return err;
    }

#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    chip::app::DnssdServer::Instance().SetExtendedDiscoveryTimeoutSecs(extDiscTimeoutSecs);
#endif

    // Init ZCL Data Model
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.dataModelProvider = CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);
    ReturnErrorOnFailure(mFactoryDataProvider.Init());
    SetDeviceInstanceInfoProvider(&mFactoryDataProvider);
    SetCommissionableDataProvider(&mFactoryDataProvider);
    SetDeviceAttestationCredentialsProvider(&mFactoryDataProvider);

    chip::Inet::EndPointStateOpenThread::OpenThreadEndpointInitParam nativeParams;
    nativeParams.lockCb                = LockOpenThreadTask;
    nativeParams.unlockCb              = UnlockOpenThreadTask;
    nativeParams.openThreadInstancePtr = chip::DeviceLayer::ThreadStackMgrImpl().OTInstance();
    initParams.endpointNativeParams    = static_cast<void *>(&nativeParams);

    // Report scheduler and timer delegate instance
    static DefaultTimerDelegate sTimerDelegate;
#if CHIP_CONFIG_SYNCHRONOUS_REPORTS_ENABLED
    static reporting::SynchronizedReportSchedulerImpl sReportScheduler(&sTimerDelegate);
#else
    static reporting::ReportSchedulerImpl sReportScheduler(&sTimerDelegate);
#endif

    initParams.reportScheduler = &sReportScheduler;

    // Manage TestEventTrigger
    static SimpleTestEventTriggerDelegate sTestEventTriggerDelegate{};
    static uint8_t sTestEventTriggerEnableKey[TestEventTriggerDelegate::kEnableKeyLength] = { 0 };
    // Read EnableKey from the factory data.
    MutableByteSpan enableKey(sTestEventTriggerEnableKey);
    err = mFactoryDataProvider.GetEnableKey(enableKey);
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERROR_APP("mFactoryDataProvider.GetEnableKey() failed!\n");
        memset(sTestEventTriggerEnableKey, 0, sizeof(sTestEventTriggerEnableKey));
    }
    VerifyOrDie(sTestEventTriggerDelegate.Init(ByteSpan(sTestEventTriggerEnableKey)) == CHIP_NO_ERROR);
    static SoftwareDiagnosticsTestEventTriggerHandler sSoftwareDiagnosticsTestEventTriggerHandler;
    err = sTestEventTriggerDelegate.AddHandler(&sSoftwareDiagnosticsTestEventTriggerHandler);
#if (OTA_SUPPORT == 1)
    static OTATestEventTriggerHandler sOTATestEventTriggerHandler;
    err = sTestEventTriggerDelegate.AddHandler(&sOTATestEventTriggerHandler);
#endif /* (OTA_SUPPORT == 1) */
#if (CHIP_CONFIG_ENABLE_ICD_SERVER == 1)
    err = sTestEventTriggerDelegate.AddHandler(&Server::GetInstance().GetICDManager());
#endif /* (CHIP_CONFIG_ENABLE_ICD_SERVER == 1) */
    initParams.testEventTriggerDelegate = &sTestEventTriggerDelegate;

    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);
    err = chip::Server::GetInstance().Init(initParams);
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERROR_APP("chip::Server::GetInstance().Init() failed\n");
        return err;
    }

    gExampleDeviceInfoProvider.SetStorageDelegate(&Server::GetInstance().GetPersistentStorage());

    ConfigurationMgr().LogDeviceConfig();

    // Open commissioning after boot if no fabric was available
    if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0)
    {
        PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));
        // Enable BLE advertisements
        err = chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
        if (err != CHIP_NO_ERROR)
        {
            LOG_ERROR_APP("failed to open commissioning window \n");
        }
        else
        {
            LOG_INFO_APP("BLE advertising started. Waiting for Pairing.\n");
        }
    }
    else
    { // try to attach to the thread network
        sHaveFabric = true;
    }

    err = PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERROR_APP("PlatformMgr().StartEventLoopTask() failed\n");
    }

    return err;
}

CHIP_ERROR AppTask::InitMatter()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = chip::Platform::MemoryInit();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERROR_APP("Platform::MemoryInit() failed\n");
    }
    else
    {
        err = PlatformMgr().InitChipStack();
        if (err != CHIP_NO_ERROR)
        {
            LOG_ERROR_APP("PlatformMgr().InitChipStack() failed\n");
        }
    }
    return err;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;
    LOG_INFO_APP("Enter AppTaskMain\n");
#if (CFG_LCD_SUPPORTED == 1)
    BSP_LCD_Clear(LCD1, LCD_COLOR_BLACK);
    UTIL_LCD_DisplayStringAt(0, 0, (uint8_t *) PRODUCT_NAME, CENTER_MODE);
    BSP_LCD_Refresh(LCD1);
#endif /* (CFG_LCD_SUPPORTED == 1) */

    CHIP_ERROR err = sAppTask.Init();

#if (CFG_LCD_SUPPORTED == 1)
    if (sHaveFabric)
    {
        char Message[20];
        snprintf(Message, sizeof(Message), "Fabric Found: %d", chip::Server::GetInstance().GetFabricTable().FabricCount());
        UTIL_LCD_DisplayStringAt(0, LINE(1), (uint8_t *) Message, LEFT_MODE);
        BSP_LCD_Refresh(LCD1);
    }
#endif /* (CFG_LCD_SUPPORTED == 1) */

#if HIGHWATERMARK
    UBaseType_t uxHighWaterMark;
    HeapStats_t HeapStatsInfo;
#endif // endif HIGHWATERMARK
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERROR_APP("App task init failed\n");
    }

    LOG_INFO_APP("App Task started\n");
    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, portMAX_DELAY);
        while (eventReceived == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
            eventReceived = xQueueReceive(sAppEventQueue, &event, 0);
        }

#if HIGHWATERMARK
        uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        LOG_INFO_APP("\x1b[34m"
                     "AppTask_stack_HighWaterMark %lu \n"
                     "\x1b[0m",
                     uxHighWaterMark);
        UNUSED(uxHighWaterMark);
        vPortGetHeapStats(&HeapStatsInfo);
        LOG_INFO_APP("\x1b[34m"
                     "AppTask_FreeRTOS_heap.freespace %u bytes\n"
                     "\x1b[0m",
                     HeapStatsInfo.xAvailableHeapSpaceInBytes);
        LOG_INFO_APP("\x1b[34m"
                     "AppTask_FreeRTOS_heap.minimum_ever_free %u bytes\n"
                     "\x1b[0m",
                     HeapStatsInfo.xMinimumEverFreeBytesRemaining);

#endif // endif HIGHWATERMARK
    }
}

#if (CFG_JOYSTICK_SUPPORTED == 1)
void AppTask::ButtonEventHandler(ButtonDesc_t * Button)
{

    if (Button->button == B1)
    { /* JOY_DOWN */
        AppEvent event;
        event.Type         = AppEvent::kEventType_Timer;
        event.Handler      = UpdateNvmEventHandler;
        sAppTask.mFunction = kFunction_SaveNvm;
        sAppTask.PostEvent(&event);
    }

    if (Button->button == B2)
    { /* JOY_SELECT */
        AppEvent event;
        event.Type         = AppEvent::kEventType_Timer;
        event.Handler      = UpdateNvmEventHandler;
        sAppTask.mFunction = kFunction_FactoryReset;
        sAppTask.PostEvent(&event);
    }

    if (Button->button == B3)
    { /* JOY_LEFT */
        AppEvent event;
        event.Type    = AppEvent::kEventType_Timer;
        event.Handler = UpdateGenericSwitchEventHandler;

        if (Button->State)
        {
            if (Button->longPressed)
            {
                sAppTask.mFunction            = kFunction_LongPress;
                sGenericSwitchLongPressActive = true;
            }
            else
            {
                sAppTask.mFunction            = kFunction_InitialPress;
                sGenericSwitchLongPressActive = false;
            }
        }
        else
        {
            if (sGenericSwitchLongPressActive)
            {
                sAppTask.mFunction            = kFunction_LongRelease;
                sGenericSwitchLongPressActive = false;
            }
            else
            {
                sAppTask.mFunction = kFunction_ShortRelease;
            }
        }
        sAppTask.PostEvent(&event);
    }
    else
    {
        return;
    }
}
#endif /* (CFG_JOYSTICK_SUPPORTED ==1) */

void AppTask::PostEvent(const AppEvent * aEvent)
{
    if (sAppEventQueue != NULL)
    {
        if (!xQueueSend(sAppEventQueue, aEvent, 1))
        {
            ChipLogError(NotSpecified, "Failed to post event to app task event queue");
        }
    }
    else
    {
        ChipLogError(NotSpecified, "Event Queue is NULL should never happen");
    }
}

void AppTask::DispatchEvent(AppEvent * aEvent)
{
    if (aEvent->Handler)
    {
        aEvent->Handler(aEvent);
    }
    else
    {
        ChipLogError(NotSpecified, "Event received with no handler. Dropping event.");
    }
}

/**
 * Update cluster status after application level changes
 */
void AppTask::UpdateClusterState(void)
{
    ChipLogProgress(NotSpecified, "UpdateClusterState");
}

void AppTask::UpdateNvmEventHandler(AppEvent * aEvent)
{
    uint8_t err = 0;

    if (sAppTask.mFunction == kFunction_SaveNvm)
    {
        err = NVM_Dump();
        if (err == 0)
        {
            LOG_INFO_APP("SAVE NVM\n");
        }
        else
        {
            LOG_ERROR_APP("Failed to SAVE NVM\n");
        }
    }
    else if (sAppTask.mFunction == kFunction_FactoryReset)
    {
        LOG_INFO_APP("FACTORY RESET\n");
        NVM_ResetFactory();
    }
}

void AppTask::UpdateGenericSwitchEventHandler(AppEvent * aEvent)
{
    if (sAppTask.mFunction == kFunction_ShortRelease)
    {
        LOG_INFO_APP("BUTTON GenericSwitchOnShortRelease");
        GenericSwitchManager::GetInstance().GenericSwitchOnShortRelease();
    }
    else if (sAppTask.mFunction == kFunction_LongRelease)
    {
        LOG_INFO_APP("BUTTON GenericSwitchOnLongRelease");
        GenericSwitchManager::GetInstance().GenericSwitchOnLongRelease();
    }
    else if (sAppTask.mFunction == kFunction_LongPress)
    {
        LOG_INFO_APP("BUTTON GenericSwitchOnLongPress");
        GenericSwitchManager::GetInstance().GenericSwitchOnLongPress();
    }
    else if (sAppTask.mFunction == kFunction_InitialPress)
    {
        LOG_INFO_APP("BUTTON GenericSwitchOnInitialPress");
        GenericSwitchManager::GetInstance().GenericSwitchOnInitialPress();
    }
}

void AppTask::MatterEventHandler(const ChipDeviceEvent * event, intptr_t)
{
    switch (event->Type)
    {
    case DeviceEventType::kServiceProvisioningChange: {
        sIsThreadProvisioned = event->ServiceProvisioningChange.IsServiceProvisioned;
        UpdateLCD();
        break;
    }

    case DeviceEventType::kThreadConnectivityChange: {
        sIsThreadEnabled = (event->ThreadConnectivityChange.Result == kConnectivity_Established);
        UpdateLCD();
        break;
    }

    case DeviceEventType::kCHIPoBLEConnectionEstablished: {
        sHaveBLEConnections = true;
        LOG_INFO_APP("kCHIPoBLEConnectionEstablished\n");
        UpdateLCD();
        break;
    }

    case DeviceEventType::kCHIPoBLEConnectionClosed: {
        sHaveBLEConnections = false;
        LOG_INFO_APP("kCHIPoBLEConnectionClosed\n");
        UpdateLCD();
        if (sFabricNeedSaved)
        {
            AppEvent event;
            event.Type         = AppEvent::kEventType_Timer;
            event.Handler      = UpdateNvmEventHandler;
            sAppTask.mFunction = kFunction_SaveNvm;
            sAppTask.PostEvent(&event);
            sFabricNeedSaved = false;
        }
        break;
    }

    case DeviceEventType::kCommissioningComplete: {
        sFabricNeedSaved = true;
        sHaveFabric      = true;

        if (sHaveBLEConnections == false)
        {
            sFabricNeedSaved = false; // put to false to avoid save in nvm 2 times
            AppEvent event;
            event.Type         = AppEvent::kEventType_Timer;
            event.Handler      = UpdateNvmEventHandler;
            sAppTask.mFunction = kFunction_SaveNvm;
            sAppTask.PostEvent(&event);
        }
        UpdateLCD();
        break;
    }
    case DeviceEventType::kFailSafeTimerExpired: {
        sFailCommissioning = true;
        UpdateLCD();
        break;
    }
    case DeviceEventType::kDnssdInitialized:

#if (OTA_SUPPORT == 1)
        InitializeOTARequestor();
#endif /* (OTA_SUPPORT == 1) */
        break;
    default:
        break;
    }
}

#if (CFG_LCD_SUPPORTED == 1)
void AppTask::UpdateLCD(void)
{
    if (sIsThreadProvisioned && sIsThreadEnabled)
    {
        UTIL_LCD_DisplayStringAt(0, LINE(4), (uint8_t *) "Network Joined", LEFT_MODE);
    }
    else if ((sIsThreadProvisioned == false) || (sIsThreadEnabled == false))
    {
        UTIL_LCD_ClearStringLine(4);
    }
    if (sHaveBLEConnections)
    {
        UTIL_LCD_ClearStringLine(1);
        BSP_LCD_Refresh(LCD1);
        UTIL_LCD_DisplayStringAt(0, LINE(1), (uint8_t *) "BLE Connected", LEFT_MODE);
    }
    if (sHaveFabric)
    {
        UTIL_LCD_ClearStringLine(1);
        BSP_LCD_Refresh(LCD1);
        UTIL_LCD_DisplayStringAt(0, LINE(1), (uint8_t *) "Fabric Created", LEFT_MODE);
    }
    if (sFailCommissioning == true)
    {
        UTIL_LCD_DisplayStringAt(0, LINE(1), (uint8_t *) "", LEFT_MODE);
        BSP_LCD_Refresh(LCD1);
        UTIL_LCD_DisplayStringAt(0, LINE(1), (uint8_t *) "Fabric Failed", LEFT_MODE);
    }
    BSP_LCD_Refresh(LCD1);
}
#else
void AppTask::UpdateLCD(void) {}
#endif /* CFG_LCD_SUPPORTED == 1 */
