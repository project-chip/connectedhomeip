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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>

#include "FreeRTOS.h"

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <examples/platform/cc32xx/CC32XXDeviceAttestationCreds.h>

#include <CHIPDeviceManager.h>
#include <DeviceCallbacks.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <platform/CHIPDeviceLayer.h>

#include <app/server/Dnssd.h>
#include <data-model-providers/codegen/Instance.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <ti/drivers/apps/Button.h>
#include <ti/drivers/apps/LED.h>

/* syscfg */
#include <ti_drivers_config.h>

extern "C" {
extern int WiFi_init();
extern void DisplayBanner();
}

#define APP_TASK_STACK_SIZE (4096)
#define APP_TASK_PRIORITY 4
#define APP_EVENT_QUEUE_SIZE 10

// Added the below three for DNS Server Initialization
using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;

using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceManager;

static TaskHandle_t sAppTaskHandle;
static QueueHandle_t sAppEventQueue;

extern LED_Handle gLedGreenHandle, gLedRedHandle;
static Button_Handle gButtonRightHandle;

AppTask AppTask::sAppTask;

static DeviceCallbacks EchoCallbacks;

int AppTask::StartAppTask()
{
    int ret = 0;

    sAppEventQueue = xQueueCreate(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent));
    if (sAppEventQueue == NULL)
    {
        PLAT_LOG("Failed to allocate app event queue");
        while (true)
            ;
    }

    // Start App task.
    if (xTaskCreate(AppTaskMain, "APP", APP_TASK_STACK_SIZE / sizeof(StackType_t), NULL, APP_TASK_PRIORITY, &sAppTaskHandle) !=
        pdPASS)
    {
        PLAT_LOG("Failed to create app task");
        while (true)
            ;
    }
    return ret;
}

int AppTask::Init()
{
    CHIP_ERROR ret;
    LED_Params ledParams;
    Button_Params buttonParams;

    cc32xxLogInit();

    /* Initial Terminal, and print Application name */
    DisplayBanner();

    // Init Chip memory management before the stack
    PLAT_LOG("Initialize Memory");
    chip::Platform::MemoryInit();

    // Initialize LEDs
    PLAT_LOG("Initialize LEDs");
    LED_init();

    LED_Params_init(&ledParams); // default PWM LED
    gLedRedHandle = LED_open(CONFIG_LED_RED, &ledParams);
    LED_setOff(gLedRedHandle);

    LED_Params_init(&ledParams); // default PWM LED
    gLedGreenHandle = LED_open(CONFIG_LED_GREEN, &ledParams);
    LED_setOff(gLedGreenHandle);

    // Initialize buttons
    PLAT_LOG("Initialize buttons");
    Button_init();

    Button_Params_init(&buttonParams);
    buttonParams.buttonEventMask   = Button_EV_CLICKED | Button_EV_LONGCLICKED;
    buttonParams.longPressDuration = 1000U; // ms
    gButtonRightHandle             = Button_open(CONFIG_BTN_RIGHT, &buttonParams);
    Button_setCallback(gButtonRightHandle, ButtonRightEventHandler);

    PLAT_LOG("Initialize Wi-Fi");
    WiFi_init();

    PLAT_LOG("Initialize CHIP stack");
    ret = PlatformMgr().InitChipStack();
    if (ret != CHIP_NO_ERROR)
    {
        PLAT_LOG("PlatformMgr().InitChipStack() failed");
        while (true)
            ;
    }

    // Init ZCL Data Model and start server
    PLAT_LOG("Initialize Server");
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.dataModelProvider = app::CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);
    chip::Server::GetInstance().Init(initParams);

    // Initialize device attestation config
    PLAT_LOG("Initialize device attestation config");
#ifdef CC32XX_ATTESTATION_CREDENTIALS
    SetDeviceAttestationCredentialsProvider(CC32XX::GetCC32XXDacProvider());
#else

    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif

    // Initialize BoltLock module
    PLAT_LOG("Initialize BoltLock");
    BoltLockMgr().Init();

    BoltLockMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    ConfigurationMgr().LogDeviceConfig();

    // QR code will be used with CHIP Tool
    PLAT_LOG("Print Onboarding Codes");
    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kOnNetwork));

    PLAT_LOG("Start CHIPDeviceManager and Start Event Loop Task");
    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();
    ret                           = deviceMgr.Init(&EchoCallbacks);
    if (ret != CHIP_NO_ERROR)
    {
        PLAT_LOG("CHIPDeviceManager::Init() failed: %s", ErrorStr(ret));
        while (1)
            ;
    }

    return 0;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;

    sAppTask.Init();

    while (true)
    {
        /* Task pend until we have stuff to do */
        if (xQueueReceive(sAppEventQueue, &event, portMAX_DELAY) == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
        }
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

void AppTask::PostEvent(const AppEvent * aEvent)
{
    if (xQueueSend(sAppEventQueue, aEvent, 0) != pdPASS)
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

    LED_setOn(gLedGreenHandle, LED_BRIGHTNESS_MAX);
    LED_startBlinking(gLedGreenHandle, 50 /* ms */, LED_BLINK_FOREVER);
    LED_setOn(gLedRedHandle, LED_BRIGHTNESS_MAX);
    LED_startBlinking(gLedRedHandle, 110 /* ms */, LED_BLINK_FOREVER);
}

void AppTask::ActionCompleted(BoltLockManager::Action_t aAction)
{
    uint8_t state;

    // if the action has been completed by the lock, update the bolt lock trait.
    // Turn on the lock LED if in a LOCKED state OR
    // Turn off the lock LED if in an UNLOCKED state.
    if (aAction == BoltLockManager::LOCK_ACTION)
    {
        PLAT_LOG("Lock completed");
        LED_stopBlinking(gLedGreenHandle);
        LED_setOn(gLedGreenHandle, LED_BRIGHTNESS_MAX);
        LED_stopBlinking(gLedRedHandle);
        LED_setOn(gLedRedHandle, LED_BRIGHTNESS_MAX);
        state = 1;
    }
    else if (aAction == BoltLockManager::UNLOCK_ACTION)
    {
        PLAT_LOG("Unlock completed");
        LED_stopBlinking(gLedGreenHandle);
        LED_setOff(gLedGreenHandle);
        LED_stopBlinking(gLedRedHandle);
        LED_setOff(gLedRedHandle);
        state = 0;
    }
    app::Clusters::OnOff::Attributes::OnOff::Set(1, state);
}

void AppTask::DispatchEvent(AppEvent * aEvent)
{
    switch (aEvent->Type)
    {
    case AppEvent::kEventType_ButtonRight:
        if (AppEvent::kAppEventButtonType_Clicked == aEvent->ButtonEvent.Type)
        {
            if (BoltLockMgr().IsUnlocked())
            {
                BoltLockMgr().InitiateAction(0, BoltLockManager::LOCK_ACTION);
            }
            else
            {
                BoltLockMgr().InitiateAction(0, BoltLockManager::UNLOCK_ACTION);
            }
        }
        break;

    case AppEvent::kEventType_AppEvent:
        if (NULL != aEvent->Handler)
        {
            // XXX: assume our caller isn't trying to crash our stack
            aEvent->Handler(aEvent);
        }
        break;

    case AppEvent::kEventType_None:
    default:
        break;
    }
}
