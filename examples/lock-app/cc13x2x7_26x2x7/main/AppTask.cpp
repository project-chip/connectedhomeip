/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Texas Instruments Incorporated
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

#include "AppTask.h"
#include "AppConfig.h"
#include "AppEvent.h"
#include <app/server/Server.h>

#include "FreeRTOS.h"

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <DeviceInfoProviderImpl.h>
#include <platform/CHIPDeviceLayer.h>

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorDriver.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <platform/cc13x2_26x2/OTAImageProcessorImpl.h>
#endif

#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>

#include <app/server/OnboardingCodesUtil.h>

#include <ti/drivers/apps/Button.h>
#include <ti/drivers/apps/LED.h>

/* syscfg */
#include <ti_drivers_config.h>

#define APP_TASK_STACK_SIZE (4096)
#define APP_TASK_PRIORITY 4
#define APP_EVENT_QUEUE_SIZE 10

using namespace ::chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

static TaskHandle_t sAppTaskHandle;
static QueueHandle_t sAppEventQueue;

static LED_Handle sAppRedHandle;
static LED_Handle sAppGreenHandle;
static Button_Handle sAppLeftHandle;
static Button_Handle sAppRightHandle;
static DeviceInfoProviderImpl sExampleDeviceInfoProvider;

AppTask AppTask::sAppTask;

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
static DefaultOTARequestor sRequestorCore;
static DefaultOTARequestorStorage sRequestorStorage;
static DefaultOTARequestorDriver sRequestorUser;
static BDXDownloader sDownloader;
static OTAImageProcessorImpl sImageProcessor;

void InitializeOTARequestor(void)
{
    // Initialize and interconnect the Requestor and Image Processor objects
    SetRequestorInstance(&sRequestorCore);

    sRequestorStorage.Init(Server::GetInstance().GetPersistentStorage());
    sRequestorCore.Init(Server::GetInstance(), sRequestorStorage, sRequestorUser, sDownloader);
    sImageProcessor.SetOTADownloader(&sDownloader);
    sDownloader.SetImageProcessorDelegate(&sImageProcessor);
    sRequestorUser.Init(&sRequestorCore, &sImageProcessor);
}
#endif

int AppTask::StartAppTask()
{
    int ret = 0;

    sAppEventQueue = xQueueCreate(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent));
    if (sAppEventQueue == NULL)
    {
        PLAT_LOG("Failed to allocate app event queue");
        while (1)
            ;
    }

    // Start App task.
    if (xTaskCreate(AppTaskMain, "APP", APP_TASK_STACK_SIZE / sizeof(StackType_t), NULL, APP_TASK_PRIORITY, &sAppTaskHandle) !=
        pdPASS)
    {
        PLAT_LOG("Failed to create app task");
        while (1)
            ;
    }
    return ret;
}

int AppTask::Init()
{
    LED_Params ledParams;
    Button_Params buttonParams;

    cc13x2_26x2LogInit();

    // Init Chip memory management before the stack
    Platform::MemoryInit();

    CHIP_ERROR ret = PlatformMgr().InitChipStack();
    if (ret != CHIP_NO_ERROR)
    {
        PLAT_LOG("PlatformMgr().InitChipStack() failed");
        while (1)
            ;
    }

    ret = ThreadStackMgr().InitThreadStack();
    if (ret != CHIP_NO_ERROR)
    {
        PLAT_LOG("ThreadStackMgr().InitThreadStack() failed");
        while (1)
            ;
    }
#if CHIP_DEVICE_CONFIG_THREAD_FTD
    ret = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router);
#else
    ret = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice);
#endif
    if (ret != CHIP_NO_ERROR)
    {
        PLAT_LOG("ConnectivityMgr().SetThreadDeviceType() failed");
        while (1)
            ;
    }

    ret = PlatformMgr().StartEventLoopTask();
    if (ret != CHIP_NO_ERROR)
    {
        PLAT_LOG("PlatformMgr().StartEventLoopTask() failed");
        while (1)
            ;
    }

    ret = ThreadStackMgrImpl().StartThreadTask();
    if (ret != CHIP_NO_ERROR)
    {
        PLAT_LOG("ThreadStackMgr().StartThreadTask() failed");
        while (1)
            ;
    }

    // Init ZCL Data Model and start server
    PLAT_LOG("Initialize Server");
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();

    // Initialize info provider
    sExampleDeviceInfoProvider.SetStorageDelegate(initParams.persistentStorageDelegate);
    SetDeviceInfoProvider(&sExampleDeviceInfoProvider);

    chip::Server::GetInstance().Init(initParams);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    // Initialize LEDs
    PLAT_LOG("Initialize LEDs");
    LED_init();

    LED_Params_init(&ledParams); // default PWM LED
    sAppRedHandle = LED_open(CONFIG_LED_RED, &ledParams);
    LED_setOff(sAppRedHandle);

    LED_Params_init(&ledParams); // default PWM LED
    sAppGreenHandle = LED_open(CONFIG_LED_GREEN, &ledParams);
    LED_setOff(sAppGreenHandle);

    // Initialize buttons
    PLAT_LOG("Initialize buttons");
    Button_init();

    Button_Params_init(&buttonParams);
    buttonParams.buttonEventMask   = Button_EV_CLICKED | Button_EV_LONGCLICKED;
    buttonParams.longPressDuration = 1000U; // ms
    sAppLeftHandle                 = Button_open(CONFIG_BTN_LEFT, &buttonParams);
    Button_setCallback(sAppLeftHandle, ButtonLeftEventHandler);

    Button_Params_init(&buttonParams);
    buttonParams.buttonEventMask   = Button_EV_CLICKED | Button_EV_LONGCLICKED;
    buttonParams.longPressDuration = 1000U; // ms
    sAppRightHandle                = Button_open(CONFIG_BTN_RIGHT, &buttonParams);
    Button_setCallback(sAppRightHandle, ButtonRightEventHandler);

    // Initialize BoltLock module
    PLAT_LOG("Initialize BoltLock");
    BoltLockMgr().Init();

    BoltLockMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    ConfigurationMgr().LogDeviceConfig();

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    InitializeOTARequestor();
#endif
    // QR code will be used with CHIP Tool
    PrintOnboardingCodes(RendezvousInformationFlags(RendezvousInformationFlag::kBLE));

    return 0;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;

    sAppTask.Init();

    while (1)
    {
        /* Task pend until we have stuff to do */
        if (xQueueReceive(sAppEventQueue, &event, portMAX_DELAY) == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
        }
    }
}

void AppTask::PostEvent(const AppEvent * aEvent)
{
    if (xQueueSend(sAppEventQueue, aEvent, 0) != pdPASS)
    {
        /* Failed to post the message */
    }
}

void AppTask::ButtonLeftEventHandler(Button_Handle handle, Button_EventMask events)
{
    AppEvent event;
    event.Type = AppEvent::kEventType_ButtonLeft;

    if (events & Button_EV_CLICKED)
    {
        event.ButtonEvent.Type = AppEvent::kAppEventButtonType_Clicked;
    }
    else if (events & Button_EV_LONGCLICKED)
    {
        event.ButtonEvent.Type = AppEvent::kAppEventButtonType_LongClicked;
    }
    // button callbacks are in ISR context
    if (xQueueSendFromISR(sAppEventQueue, &event, NULL) != pdPASS)
    {
        /* Failed to post the message */
    }
}

void AppTask::ButtonRightEventHandler(Button_Handle handle, Button_EventMask events)
{
    AppEvent event;
    event.Type = AppEvent::kEventType_ButtonRight;

    if (events & Button_EV_CLICKED)
    {
        event.ButtonEvent.Type = AppEvent::kAppEventButtonType_Clicked;
    }
    else if (events & Button_EV_LONGCLICKED)
    {
        event.ButtonEvent.Type = AppEvent::kAppEventButtonType_LongClicked;
    }
    // button callbacks are in ISR context
    if (xQueueSendFromISR(sAppEventQueue, &event, NULL) != pdPASS)
    {
        /* Failed to post the message */
    }
}

void AppTask::ActionInitiated(BoltLockManager::Action_t aAction, int32_t aActor)
{
    // If the action has been initiated by the lock, update the bolt lock trait
    // and start flashing the LEDs rapidly to indicate action initiation.
    if (aAction == BoltLockManager::LOCK_ACTION)
    {
        PLAT_LOG("Lock initiated");
        ; // TODO
    }
    else if (aAction == BoltLockManager::UNLOCK_ACTION)
    {
        PLAT_LOG("Unlock initiated");
        ; // TODO
    }

    LED_setOn(sAppGreenHandle, LED_BRIGHTNESS_MAX);
    LED_startBlinking(sAppGreenHandle, 50 /* ms */, LED_BLINK_FOREVER);
    LED_setOn(sAppRedHandle, LED_BRIGHTNESS_MAX);
    LED_startBlinking(sAppRedHandle, 110 /* ms */, LED_BLINK_FOREVER);
}

void AppTask::ActionCompleted(BoltLockManager::Action_t aAction)
{
    // if the action has been completed by the lock, update the bolt lock trait.
    // Turn on the lock LED if in a LOCKED state OR
    // Turn off the lock LED if in an UNLOCKED state.
    if (aAction == BoltLockManager::LOCK_ACTION)
    {
        PLAT_LOG("Lock completed");
        LED_stopBlinking(sAppGreenHandle);
        LED_setOn(sAppGreenHandle, LED_BRIGHTNESS_MAX);
        LED_stopBlinking(sAppRedHandle);
        LED_setOn(sAppRedHandle, LED_BRIGHTNESS_MAX);
    }
    else if (aAction == BoltLockManager::UNLOCK_ACTION)
    {
        PLAT_LOG("Unlock completed");
        LED_stopBlinking(sAppGreenHandle);
        LED_setOff(sAppGreenHandle);
        LED_stopBlinking(sAppRedHandle);
        LED_setOff(sAppRedHandle);
    }
}

void AppTask::DispatchEvent(AppEvent * aEvent)
{
    switch (aEvent->Type)
    {
    case AppEvent::kEventType_ButtonLeft:
        if (AppEvent::kAppEventButtonType_Clicked == aEvent->ButtonEvent.Type)
        {
            if (!BoltLockMgr().IsUnlocked())
            {
                BoltLockMgr().InitiateAction(0, BoltLockManager::UNLOCK_ACTION);
            }
        }
        else if (AppEvent::kAppEventButtonType_LongClicked == aEvent->ButtonEvent.Type)
        {
            chip::Server::GetInstance().ScheduleFactoryReset();
        }
        break;

    case AppEvent::kEventType_ButtonRight:
        if (AppEvent::kAppEventButtonType_Clicked == aEvent->ButtonEvent.Type)
        {
            if (BoltLockMgr().IsUnlocked())
            {
                BoltLockMgr().InitiateAction(0, BoltLockManager::LOCK_ACTION);
            }
        }
        else if (AppEvent::kAppEventButtonType_LongClicked == aEvent->ButtonEvent.Type)
        {
            // Enable BLE advertisements
            if (!ConnectivityMgr().IsBLEAdvertisingEnabled())
            {
                if (Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow() == CHIP_NO_ERROR)
                {
                    PLAT_LOG("Enabled BLE Advertisements");
                }
                else
                {
                    PLAT_LOG("OpenBasicCommissioningWindow() failed");
                }
            }
            else
            {
                // Disable BLE advertisements
                ConnectivityMgr().SetBLEAdvertisingEnabled(false);
                PLAT_LOG("Disabled BLE Advertisements");
            }
        }
        break;

    case AppEvent::kEventType_AppEvent:
        if (NULL != aEvent->Handler)
        {
            aEvent->Handler(aEvent);
        }
        break;

    case AppEvent::kEventType_None:
    default:
        break;
    }
}
