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
#include <app/server/Dnssd.h>
#include <app/server/Server.h>

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-id.h>

#include "FreeRTOS.h"
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <app/EventLogging.h>
#include <app/util/af-types.h>
#include <app/util/af.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <platform/CHIPDeviceLayer.h>

#include <app/server/OnboardingCodesUtil.h>

#include <ti/drivers/apps/Button.h>
#include <ti/drivers/apps/LED.h>

/* syscfg */
#include <ti_drivers_config.h>

#define APP_TASK_STACK_SIZE (4096)
#define APP_TASK_PRIORITY 4
#define APP_EVENT_QUEUE_SIZE 10

#define PCC_CLUSTER_ENDPOINT 1
#define ONOFF_CLUSTER_ENDPOINT 1
#define EXTENDED_DISCOVERY_TIMEOUT_SEC 20

using namespace chip;
using namespace chip::app;
using namespace chip::Credentials;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters;

static TaskHandle_t sAppTaskHandle;
static QueueHandle_t sAppEventQueue;

static LED_Handle sAppRedHandle;
static LED_Handle sAppGreenHandle;
static Button_Handle sAppLeftHandle;
static Button_Handle sAppRightHandle;

AppTask AppTask::sAppTask;

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
    chip::Platform::MemoryInit();

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

    ret = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router);
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

#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    chip::app::DnssdServer::Instance().SetExtendedDiscoveryTimeoutSecs(EXTENDED_DISCOVERY_TIMEOUT_SEC);
#endif

    // Init ZCL Data Model and start server
    PLAT_LOG("Initialize Server");
    chip::Server::GetInstance().Init();

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
    buttonParams.buttonEventMask   = Button_EV_CLICKED | Button_EV_LONGPRESSED;
    buttonParams.longPressDuration = 5000U; // ms
    sAppLeftHandle                 = Button_open(CONFIG_BTN_LEFT, &buttonParams);
    Button_setCallback(sAppLeftHandle, ButtonLeftEventHandler);

    Button_Params_init(&buttonParams);
    buttonParams.buttonEventMask   = Button_EV_CLICKED;
    buttonParams.longPressDuration = 1000U; // ms
    sAppRightHandle                = Button_open(CONFIG_BTN_RIGHT, &buttonParams);
    Button_setCallback(sAppRightHandle, ButtonRightEventHandler);

    // Initialize Pump module
    PLAT_LOG("Initialize Pump");
    PumpMgr().Init();

    PumpMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    ConfigurationMgr().LogDeviceConfig();

    // QR code will be used with CHIP Tool
    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));

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
    else if (events & Button_EV_LONGPRESSED)
    {
        event.ButtonEvent.Type = AppEvent::kAppEventButtonType_LongPressed;
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
    // button callbacks are in ISR context
    if (xQueueSendFromISR(sAppEventQueue, &event, NULL) != pdPASS)
    {
        /* Failed to post the message */
    }
}

void AppTask::ActionInitiated(PumpManager::Action_t aAction, int32_t aActor)
{
    // If the action has been initiated by the pump, update the pump trait
    // and start flashing the LEDs rapidly to indicate action initiation.
    if (aAction == PumpManager::START_ACTION)
    {
        PLAT_LOG("Pump start initiated");
        ; // TODO
    }
    else if (aAction == PumpManager::STOP_ACTION)
    {
        PLAT_LOG("Stop initiated");
        ; // TODO
    }

    LED_setOn(sAppGreenHandle, LED_BRIGHTNESS_MAX);
    LED_startBlinking(sAppGreenHandle, 50 /* ms */, LED_BLINK_FOREVER);
    LED_setOn(sAppRedHandle, LED_BRIGHTNESS_MAX);
    LED_startBlinking(sAppRedHandle, 110 /* ms */, LED_BLINK_FOREVER);
}

void AppTask::ActionCompleted(PumpManager::Action_t aAction, int32_t aActor)
{
    // if the action has been completed by the pump, update the pump trait.
    // Turn on the pump state LED if in a STARTED state OR
    // Turn off the pump state LED if in an STOPPED state.
    if (aAction == PumpManager::START_ACTION)
    {
        PLAT_LOG("Pump start completed");
        LED_stopBlinking(sAppGreenHandle);
        LED_setOn(sAppGreenHandle, LED_BRIGHTNESS_MAX);
        LED_stopBlinking(sAppRedHandle);
        LED_setOn(sAppRedHandle, LED_BRIGHTNESS_MAX);
    }
    else if (aAction == PumpManager::STOP_ACTION)
    {
        PLAT_LOG("Pump stop completed");
        LED_stopBlinking(sAppGreenHandle);
        LED_setOff(sAppGreenHandle);
        LED_stopBlinking(sAppRedHandle);
        LED_setOff(sAppRedHandle);
    }
    if (aActor == AppEvent::kEventType_ButtonLeft)
    {
        sAppTask.UpdateClusterState();
    }
}

void AppTask::DispatchEvent(AppEvent * aEvent)
{
    switch (aEvent->Type)
    {
    case AppEvent::kEventType_ButtonRight:
        if (AppEvent::kAppEventButtonType_Clicked == aEvent->ButtonEvent.Type)
        {
            // Toggle Pump state
            if (!PumpMgr().IsStopped())
            {
                PumpMgr().InitiateAction(0, PumpManager::STOP_ACTION);
            }
            else
            {
                PumpMgr().InitiateAction(0, PumpManager::START_ACTION);
            }
        }
        break;

    case AppEvent::kEventType_ButtonLeft:
        if (AppEvent::kAppEventButtonType_Clicked == aEvent->ButtonEvent.Type)
        {
            // Post event for demonstration purposes
            sAppTask.PostEvents();

            // Toggle BLE advertisements
            if (!ConnectivityMgr().IsBLEAdvertisingEnabled())
            {
                if (chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow() == CHIP_NO_ERROR)
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
        else if (AppEvent::kAppEventButtonType_LongPressed == aEvent->ButtonEvent.Type)
        {
            ConfigurationMgr().InitiateFactoryReset();
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

void AppTask::InitOnOffClusterState()
{

    EmberStatus status;

    ChipLogProgress(NotSpecified, "Init On/Off clusterstate");

    // Write false as pump always boots in stopped mode
    status = OnOff::Attributes::OnOff::Set(ONOFF_CLUSTER_ENDPOINT, false);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ERR: Init On/Off state  %" PRIx8, status);
    }
}

void AppTask::InitPCCClusterState() {}

void AppTask::UpdateClusterState()
{
    EmberStatus status;

    ChipLogProgress(NotSpecified, "UpdateClusterState");

    // Write the new values
    bool onOffState = !PumpMgr().IsStopped();
    status          = OnOff::Attributes::OnOff::Set(ONOFF_CLUSTER_ENDPOINT, onOffState);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ERR: Updating On/Off state  %" PRIx8, status);
    }

    int16_t maxPressure = PumpMgr().GetMaxPressure();
    status              = PumpConfigurationAndControl::Attributes::MaxPressure::Set(PCC_CLUSTER_ENDPOINT, maxPressure);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ERR: Updating MaxPressure  %" PRIx8, status);
    }

    uint16_t maxSpeed = PumpMgr().GetMaxSpeed();
    status            = PumpConfigurationAndControl::Attributes::MaxSpeed::Set(PCC_CLUSTER_ENDPOINT, maxSpeed);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ERR: Updating MaxSpeed  %" PRIx8, status);
    }

    uint16_t maxFlow = PumpMgr().GetMaxFlow();
    status           = PumpConfigurationAndControl::Attributes::MaxFlow::Set(PCC_CLUSTER_ENDPOINT, maxFlow);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ERR: Updating MaxFlow  %" PRIx8, status);
    }

    int16_t minConstPress = PumpMgr().GetMinConstPressure();
    status                = PumpConfigurationAndControl::Attributes::MinConstPressure::Set(PCC_CLUSTER_ENDPOINT, minConstPress);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ERR: Updating MinConstPressure  %" PRIx8, status);
    }

    int16_t maxConstPress = PumpMgr().GetMaxConstPressure();
    status                = PumpConfigurationAndControl::Attributes::MaxConstPressure::Set(PCC_CLUSTER_ENDPOINT, maxConstPress);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ERR: Updating MaxConstPressure  %" PRIx8, status);
    }

    int16_t minCompPress = PumpMgr().GetMinCompPressure();
    status               = PumpConfigurationAndControl::Attributes::MinCompPressure::Set(PCC_CLUSTER_ENDPOINT, minCompPress);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ERR: Updating MinCompPressure  %" PRIx8, status);
    }

    int16_t maxCompPress = PumpMgr().GetMaxCompPressure();
    status               = PumpConfigurationAndControl::Attributes::MaxCompPressure::Set(PCC_CLUSTER_ENDPOINT, maxCompPress);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ERR: Updating MaxCompPressure  %" PRIx8, status);
    }

    uint16_t minConstSpeed = PumpMgr().GetMinConstSpeed();
    status                 = PumpConfigurationAndControl::Attributes::MinConstSpeed::Set(PCC_CLUSTER_ENDPOINT, minConstSpeed);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ERR: Updating MinConstSpeed  %" PRIx8, status);
    }

    uint16_t maxConstSpeed = PumpMgr().GetMaxConstSpeed();
    status                 = PumpConfigurationAndControl::Attributes::MaxConstSpeed::Set(PCC_CLUSTER_ENDPOINT, maxConstSpeed);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ERR: Updating MaxConstSpeed  %" PRIx8, status);
    }

    uint16_t minConstFlow = PumpMgr().GetMinConstFlow();
    status                = PumpConfigurationAndControl::Attributes::MinConstFlow::Set(PCC_CLUSTER_ENDPOINT, minConstFlow);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ERR: Updating MinConstFlow  %" PRIx8, status);
    }

    uint16_t maxConstFlow = PumpMgr().GetMaxConstFlow();
    status                = PumpConfigurationAndControl::Attributes::MaxConstFlow::Set(PCC_CLUSTER_ENDPOINT, maxConstFlow);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ERR: Updating MaxConstFlow  %" PRIx8, status);
    }

    int16_t minConstTemp = PumpMgr().GetMinConstTemp();
    status               = PumpConfigurationAndControl::Attributes::MinConstTemp::Set(PCC_CLUSTER_ENDPOINT, minConstTemp);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ERR: Updating MinConstTemp  %" PRIx8, status);
    }

    int16_t maxConstTemp = PumpMgr().GetMaxConstTemp();
    status               = PumpConfigurationAndControl::Attributes::MaxConstTemp::Set(PCC_CLUSTER_ENDPOINT, maxConstTemp);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ERR: Updating MaxConstTemp  %" PRIx8, status);
    }
}

void AppTask::PostEvents()
{
    // Example on posting events - here we post the general fault event on endpoints with PCC Server enabled
    for (auto endpoint : EnabledEndpointsWithServerCluster(PumpConfigurationAndControl::Id))
    {
        PumpConfigurationAndControl::Events::GeneralFault::Type event;
        EventNumber eventNumber;

        ChipLogProgress(Zcl, "AppTask: Post PCC GeneralFault event");
        // Using default priority for the event
        if (CHIP_NO_ERROR != LogEvent(event, endpoint, eventNumber))
        {
            ChipLogError(Zcl, "AppTask: Failed to record GeneralFault event");
        }
    }
}
