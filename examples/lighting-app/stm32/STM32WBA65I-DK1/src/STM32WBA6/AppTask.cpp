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
#include "app_common.h"
#include "app_thread.h"
#include "stm32_lpm.h"
#include "dbg_trace.h"
#include "cmsis_os2.h"
#include "AppEvent.h"
#include "AppTask.h"
#include "flash_wb.h"

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
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>
#include <platform/CHIPDeviceLayer.h>

#if CHIP_ENABLE_OPENTHREAD
#include <platform/OpenThread/GenericNetworkCommissioningThreadDriver.h>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>
#endif

#include "stm32wba65i_discovery.h"
#include "app_conf.h"
#include "app_bsp.h"

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
#define NVM_TIMEOUT 1000  // timer to handle PB to save data in nvm or do a factory reset
#define STM32_LIGHT_ENDPOINT_ID 1

static QueueHandle_t sAppEventQueue;


const osThreadAttr_t AppTask_attr =
{
  .name = APPTASK_NAME,
  .attr_bits = APP_ATTR_BITS,
  .cb_mem = APP_CB_MEM,
  .cb_size = APP_CB_SIZE,
  .stack_mem = APP_STACK_MEM,
  .stack_size = APP_STACK_SIZE,
  .priority =  APP_PRIORITY,
};

static bool sIsThreadProvisioned = false;
static bool sIsThreadEnabled = false;
static bool sHaveBLEConnections = false;
static bool sFabricNeedSaved = false;
static bool sFailCommissioning = false;
static bool sHaveFabric = false;

chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

#if CHIP_ENABLE_OPENTHREAD
Clusters::NetworkCommissioning::InstanceAndDriver<DeviceLayer::NetworkCommissioning::GenericThreadDriver>
    sThreadNetworkDriver(0 /*endpointId*/);
#endif // CHIP_ENABLE_OPENTHREAD

CHIP_ERROR AppTask::StartAppTask() {
    sAppEventQueue = xQueueCreate(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent));
    if (sAppEventQueue == NULL) {
        APP_DBG("Failed to allocate app event queue\n");
        return CHIP_ERROR_NO_MEMORY;
    }

    // Start App task.
    osThreadNew(AppTaskMain, NULL, &AppTask_attr);

    return CHIP_NO_ERROR;
}

void LockOpenThreadTask(void) {
	APP_THREAD_LockThreadStack();
}

void UnlockOpenThreadTask(void) {
	APP_THREAD_UnLockThreadStack();
}

CHIP_ERROR AppTask::Init() {

    CHIP_ERROR err = CHIP_NO_ERROR;

    /* Setup button handler */
    APPE_PushButtonSetReceiveCb(ButtonEventHandler);

    ThreadStackMgr().InitThreadStack();

    ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router);
    sThreadNetworkDriver.Init();
    PlatformMgr().AddEventHandler(MatterEventHandler, 0);

    err = LightingMgr().Init();
    if (err != CHIP_NO_ERROR) {
        APP_DBG("LightingMgr().Init() failed\n");
        return err;
    }
    LightingMgr().SetCallbacks(ActionInitiated, ActionCompleted);

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
    nativeParams.lockCb = LockOpenThreadTask;
    nativeParams.unlockCb = UnlockOpenThreadTask;
    nativeParams.openThreadInstancePtr = chip::DeviceLayer::ThreadStackMgrImpl().OTInstance();
    initParams.endpointNativeParams = static_cast<void*>(&nativeParams);
    chip::Server::GetInstance().Init(initParams);

    gExampleDeviceInfoProvider.SetStorageDelegate(&Server::GetInstance().GetPersistentStorage());
    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    ConfigurationMgr().LogDeviceConfig();
    
    // Open commissioning after boot if no fabric was available
    if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
        PrintOnboardingCodes(
                chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));
        // Enable BLE advertisements
        chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
        APP_DBG("BLE advertising started. Waiting for Pairing.\n");
    } else {  // try to attach to the thread network
        sHaveFabric = true;
    }



    err = PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR) {
        APP_DBG("PlatformMgr().StartEventLoopTask() failed\n");
    }

    return err;
}

CHIP_ERROR AppTask::InitMatter() {
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = chip::Platform::MemoryInit();
    if (err != CHIP_NO_ERROR) {
        APP_DBG("Platform::MemoryInit() failed\n");
    } else {
        err = PlatformMgr().InitChipStack();
        if (err != CHIP_NO_ERROR) {
            APP_DBG("PlatformMgr().InitChipStack() failed\n");
        }
    }
    return err;
}

void AppTask::AppTaskMain(void *pvParameter) {
    AppEvent event;
#if (CFG_LCD_SUPPORTED == 1)
    APP_BSP_LcdInit();
    BSP_LCD_Clear(LCD1,LCD_COLOR_BLACK);
    UTIL_LCD_DisplayStringAt(0, 0, (uint8_t*) PRODUCT_NAME, CENTER_MODE);
    BSP_LCD_Refresh(LCD1);
#endif /* (CFG_LCD_SUPPORTED == 1) */

    CHIP_ERROR err = sAppTask.Init();

#if (CFG_LCD_SUPPORTED == 1)
    if(sHaveFabric)
    {
      	char Message[20];
      	snprintf(Message, sizeof(Message), "Fabric Found: %d",
       	chip::Server::GetInstance().GetFabricTable().FabricCount());
       	UTIL_LCD_DisplayStringAt(0, LINE(1), (uint8_t*) Message, LEFT_MODE);
       	BSP_LCD_Refresh(LCD1);
    }
#endif /* (CFG_LCD_SUPPORTED == 1) */

#if HIGHWATERMARK
    UBaseType_t uxHighWaterMark;
    HeapStats_t HeapStatsInfo;
#endif // endif HIGHWATERMARK
    if (err != CHIP_NO_ERROR) {
        APP_DBG("App task init failed\n");
    }

    APP_DBG("App Task started\n");
    while (true) {

        BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, portMAX_DELAY);
        while (eventReceived == pdTRUE) {
            sAppTask.DispatchEvent(&event);
            eventReceived = xQueueReceive(sAppEventQueue, &event, 0);
        }
#if HIGHWATERMARK
        uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
        APP_DBG ("\x1b[34m" "AppTask_stack_HighWaterMark %lu \n" "\x1b[0m",uxHighWaterMark);
        vPortGetHeapStats(&HeapStatsInfo);
        APP_DBG ("\x1b[34m" "AppTask_FreeRTOS_heap.freespace %u bytes\n" "\x1b[0m",HeapStatsInfo.xAvailableHeapSpaceInBytes);
        APP_DBG ("\x1b[34m" "AppTask_FreeRTOS_heap.minimum_ever_free %u bytes\n" "\x1b[0m",HeapStatsInfo.xMinimumEverFreeBytesRemaining);

#endif // endif HIGHWATERMARK
    }
}

void AppTask::LightingActionEventHandler(AppEvent *aEvent) {
    LightingManager::Action_t action;

    if (aEvent->Type == AppEvent::kEventType_Button) {
        // Toggle light
        if (LightingMgr().IsTurnedOn()) {
            action = LightingManager::OFF_ACTION;
        } else {
            action = LightingManager::ON_ACTION;
        }

        sAppTask.mSyncClusterToButtonAction = true;
        LightingMgr().InitiateAction(action, 0, 0, 0);
    }
    if (aEvent->Type == AppEvent::kEventType_Level && aEvent->ButtonEvent.Action != 0) {
        // Toggle Dimming of light between 2 fixed levels
        uint8_t val = 0x0;
        val = LightingMgr().GetLevel() == 0x7f ? 0x1 : 0x7f;
        action = LightingManager::LEVEL_ACTION;

        sAppTask.mSyncClusterToButtonAction = true;
        LightingMgr().InitiateAction(action, 0, 1, &val);
    }
}

void AppTask::ButtonEventHandler(ButtonDesc_t *Button) {


    if (Button->button == B1) { /* JOY_UP */
            // Hand off to Functionality handler - depends on duration of press
    	 AppEvent event;
    	 event.Type = AppEvent::kEventType_Timer;
    	 event.Handler = UpdateNvmEventHandler;
    	 sAppTask.mFunction = kFunction_SaveNvm;
    	 sAppTask.PostEvent(&event);
    }
    if (Button->button == B2) {/* JOY_SELECT */
    	 AppEvent event;
    	 event.Type = AppEvent::kEventType_Timer;
    	 event.Handler = UpdateNvmEventHandler;
    	 sAppTask.mFunction = kFunction_FactoryReset;
    	 sAppTask.PostEvent(&event);
    }
    else {
        return;
    }

}


void AppTask::ActionInitiated(LightingManager::Action_t aAction) {
    // Placeholder for light action
    if (aAction == LightingManager::ON_ACTION) {
        APP_DBG("Light goes on\n");

#if (CFG_LCD_SUPPORTED == 1)
        UTIL_LCD_ClearStringLine(2);
        char Message[11];
        snprintf(Message, sizeof(Message), "LED ON %d", LightingMgr().GetLevel());
        UTIL_LCD_DisplayStringAt(0, LINE(2), (uint8_t*) Message, CENTER_MODE);
        BSP_LCD_Refresh(LCD1);

#endif /* (CFG_LCD_SUPPORTED == 1) */
        BSP_LED_On(LED_GREEN);

    } else if (aAction == LightingManager::OFF_ACTION) {
        APP_DBG("Light goes off\n ");

#if (CFG_LCD_SUPPORTED == 1)
        UTIL_LCD_ClearStringLine(2);
        BSP_LCD_Refresh(LCD1);
#endif /* (CFG_LCD_SUPPORTED == 1) */
        BSP_LED_Off(LED_GREEN);

    } else if (aAction == LightingManager::LEVEL_ACTION) {
        if (LightingMgr().IsTurnedOn()) {

#if (CFG_LCD_SUPPORTED == 1)
            UTIL_LCD_ClearStringLine(2);
            char Message[11];
            snprintf(Message, sizeof(Message), "LED ON %d", LightingMgr().GetLevel());
            UTIL_LCD_DisplayStringAt(0, LINE(2), (uint8_t*) Message, CENTER_MODE);
            BSP_LCD_Refresh(LCD1);
#endif /* (CFG_LCD_SUPPORTED == 1) */

        	APP_DBG("Update level control %d\n", LightingMgr().GetLevel());
        }
    }

}

void AppTask::ActionCompleted(LightingManager::Action_t aAction) {
    // Placeholder for light action completed
    if (aAction == LightingManager::ON_ACTION) {
        APP_DBG("Light action on completed\n");
    } else if (aAction == LightingManager::OFF_ACTION) {
        APP_DBG("Light action off completed\n");
    }
    if (sAppTask.mSyncClusterToButtonAction) {
        sAppTask.UpdateClusterState();
        sAppTask.mSyncClusterToButtonAction = false;
    }
}

void AppTask::PostEvent(const AppEvent *aEvent) {
    if (sAppEventQueue != NULL) {
        if (!xQueueSend(sAppEventQueue, aEvent, 1)) {
            ChipLogError(NotSpecified, "Failed to post event to app task event queue");
        }
    } else {
        ChipLogError(NotSpecified, "Event Queue is NULL should never happen");
    }
}

void AppTask::DispatchEvent(AppEvent *aEvent) {
    if (aEvent->Handler) {
        aEvent->Handler(aEvent);
    } else {
        ChipLogError(NotSpecified, "Event received with no handler. Dropping event.");
    }
}

/**
 * Update cluster status after application level changes
 */
void AppTask::UpdateClusterState(void) {
    ChipLogProgress(NotSpecified, "UpdateClusterState");
    // Write the new on/off value
    Protocols::InteractionModel::Status status;

    status = Clusters::OnOff::Attributes::OnOff::Set(
             STM32_LIGHT_ENDPOINT_ID, LightingMgr().IsTurnedOn());
    if(status != Protocols::InteractionModel::Status::Success){
    	ChipLogError(NotSpecified, "ERR: updating on/off %x", static_cast<unsigned int>(status));
    }

    // Write new level value
    status = Clusters::LevelControl::Attributes::CurrentLevel::Set(
    STM32_LIGHT_ENDPOINT_ID, LightingMgr().GetLevel());
    if (status != Protocols::InteractionModel::Status::Success){
        ChipLogError(NotSpecified, "ERR: updating level %x", static_cast<unsigned int>(status));
    }
}

void AppTask::UpdateNvmEventHandler(AppEvent *aEvent) {
    uint8_t err = 0;

    if (sAppTask.mFunction == kFunction_SaveNvm) {
        err = NM_Dump();
        if (err == 0) {
            APP_DBG("SAVE NVM\n");
        } else {
            APP_DBG("Failed to SAVE NVM\n");
        }
    } else if (sAppTask.mFunction == kFunction_FactoryReset) {
        APP_DBG("FACTORY RESET\n");
        NM_ResetFactory();
    }
}

void AppTask::MatterEventHandler(const ChipDeviceEvent *event, intptr_t) {
    switch (event->Type) {
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
        APP_DBG("kCHIPoBLEConnectionEstablished\n");
        UpdateLCD();
        break;
    }

    case DeviceEventType::kCHIPoBLEConnectionClosed: {
        sHaveBLEConnections = false;
        APP_DBG("kCHIPoBLEConnectionClosed\n");
        UpdateLCD();
        if (sFabricNeedSaved) {
            AppEvent event;
            event.Type = AppEvent::kEventType_Timer;
            event.Handler = UpdateNvmEventHandler;
            sAppTask.mFunction = kFunction_SaveNvm;
            sAppTask.PostEvent(&event);
            sFabricNeedSaved = false;
        }
        break;
    }

    case DeviceEventType::kCommissioningComplete: {
        sFabricNeedSaved = true;
        sHaveFabric = true;
        // check if ble is on, since before save in nvm we need to stop m0, Better to write in nvm when m0 is less busy
        if (sHaveBLEConnections == false) {
            sFabricNeedSaved = false; // put to false to avoid save in nvm 2 times
            AppEvent event;
            event.Type = AppEvent::kEventType_Timer;
            event.Handler = UpdateNvmEventHandler;
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
void AppTask::UpdateLCD(void) {
    if (sIsThreadProvisioned && sIsThreadEnabled) {
        UTIL_LCD_DisplayStringAt(0, LINE(4), (uint8_t*) "Network Joined", LEFT_MODE);
    } else if ((sIsThreadProvisioned == false) || (sIsThreadEnabled == false)) {
        UTIL_LCD_ClearStringLine(4);
    }
    if (sHaveBLEConnections) {
        UTIL_LCD_ClearStringLine(1);
        BSP_LCD_Refresh(LCD1);
        UTIL_LCD_DisplayStringAt(0, LINE(1), (uint8_t*) "BLE Connected", LEFT_MODE);
    }
    if (sHaveFabric) {
        UTIL_LCD_ClearStringLine(1);
        BSP_LCD_Refresh(LCD1);
        UTIL_LCD_DisplayStringAt(0, LINE(1), (uint8_t*) "Fabric Created", LEFT_MODE);
    }
    if (sFailCommissioning == true) {
        UTIL_LCD_DisplayStringAt(0, LINE(1), (uint8_t*) "", LEFT_MODE);
        BSP_LCD_Refresh(LCD1);
        UTIL_LCD_DisplayStringAt(0, LINE(1), (uint8_t*) "Fabric Failed", LEFT_MODE);
    }
    BSP_LCD_Refresh(LCD1);
}
#else
void AppTask::UpdateLCD(void)
{
}
#endif /* CFG_LCD_SUPPORTED == 1 */





