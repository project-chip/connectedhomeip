/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include "ButtonHandler.h"
#include "DataModelHandler.h"
#include "LEDWidget.h"
#include "OnboardingCodesUtil.h"
#include "Server.h"
#include "qrcodegen.h"
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/server/Mdns.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <assert.h>
#include <cy_wcm.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#define FACTORY_RESET_TRIGGER_TIMEOUT 3000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#define APP_TASK_STACK_SIZE (4096)
#define APP_TASK_PRIORITY 2
#define APP_EVENT_QUEUE_SIZE 10

namespace {

TimerHandle_t sFunctionTimer; // FreeRTOS app sw timer.

static TaskHandle_t sAppTaskHandle;
static QueueHandle_t sAppEventQueue;

static LEDWidget sStatusLED;
static LEDWidget sLockLED;

static bool sIsWiFiStationProvisioned = false;
static bool sIsWiFiStationEnabled     = false;
static bool sIsWiFiStationConnected   = false;
static bool sHaveBLEConnections       = false;
static bool sHaveServiceConnectivity  = false;

StackType_t appStack[APP_TASK_STACK_SIZE / sizeof(StackType_t)];
StaticTask_t appTaskStruct;
} // namespace
using namespace chip::TLV;
using namespace ::chip::DeviceLayer;

AppTask AppTask::sAppTask;

int AppTask::StartAppTask()
{
    int err = CHIP_ERROR_SENTINEL;

    sAppEventQueue = xQueueCreate(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent));
    if (sAppEventQueue == NULL)
    {
        P6_LOG("Failed to allocate app event queue");
        appError(err);
    }

    // Start App task.
    sAppTaskHandle = xTaskCreateStatic(AppTaskMain, APP_TASK_NAME, ArraySize(appStack), NULL, 1, appStack, &appTaskStruct);
    if (sAppTaskHandle != NULL)
    {
        err = CHIP_NO_ERROR;
    }

    return err;
}

int AppTask::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Register the callback to init the MDNS server when connectivity is available
    PlatformMgr().AddEventHandler(
        [](const ChipDeviceEvent * event, intptr_t arg) {
            // Restart the server whenever an ip address is renewed
            if (event->Type == DeviceEventType::kInternetConnectivityChange)
            {
                if (event->InternetConnectivityChange.IPv4 == kConnectivity_Established ||
                    event->InternetConnectivityChange.IPv6 == kConnectivity_Established)
                {
                    chip::app::Mdns::StartServer();
                }
            }
        },
        0);
    // Init ZCL Data Model
    InitServer();

    // Initialise WSTK buttons PB0 and PB1 (including debounce).
    ButtonHandler::Init();

    // Create FreeRTOS sw timer for Function Selection.
    sFunctionTimer = xTimerCreate("FnTmr",          // Just a text name, not used by the RTOS kernel
                                  1,                // == default timer period (mS)
                                  false,            // no timer reload (==one-shot)
                                  (void *) this,    // init timer id = app task obj context
                                  TimerEventHandler // timer callback handler
    );
    if (sFunctionTimer == NULL)
    {
        P6_LOG("funct timer create failed");
        appError(err);
    }

    P6_LOG("Current Firmware Version: %d", CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION);
    err = BoltLockMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        P6_LOG("BoltLockMgr().Init() failed");
        appError(err);
    }

    BoltLockMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    // Initialize LEDs
    sStatusLED.Init(SYSTEM_STATE_LED);
    sLockLED.Init(LOCK_STATE_LED);
    sLockLED.Set(!BoltLockMgr().IsUnlocked());
    UpdateClusterState();

    ConfigurationMgr().LogDeviceConfig();

    // Print setup info
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));

    return err;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    int err;
    AppEvent event;

    err = sAppTask.Init();
    if (err != CHIP_NO_ERROR)
    {
        P6_LOG("AppTask.Init() failed");
        appError(err);
    }

    P6_LOG("App Task started");

    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, pdMS_TO_TICKS(10));
        while (eventReceived == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
            eventReceived = xQueueReceive(sAppEventQueue, &event, 0);
        }
        // Collect connectivity and configuration state from the CHIP stack. Because
        // the CHIP event loop is being run in a separate task, the stack must be
        // locked while these values are queried.  However we use a non-blocking
        // lock request (TryLockCHIPStack()) to avoid blocking other UI activities
        // when the CHIP task is busy (e.g. with a long crypto operation).
        if (PlatformMgr().TryLockChipStack())
        {
            sIsWiFiStationEnabled     = ConnectivityMgr().IsWiFiStationEnabled();
            sIsWiFiStationConnected   = ConnectivityMgr().IsWiFiStationConnected();
            sIsWiFiStationProvisioned = ConnectivityMgr().IsWiFiStationProvisioned();
            sHaveBLEConnections       = (ConnectivityMgr().NumBLEConnections() != 0);
            sHaveServiceConnectivity  = ConnectivityMgr().HaveServiceConnectivity();
            PlatformMgr().UnlockChipStack();
        }

        // Update the status LED if factory reset has not been initiated.
        //
        // If system has "full connectivity", keep the LED On constantly.
        //
        // If thread and service provisioned, but not attached to the thread network
        // yet OR no connectivity to the service OR subscriptions are not fully
        // established THEN blink the LED Off for a short period of time.
        //
        // If the system has ble connection(s) uptill the stage above, THEN blink
        // the LEDs at an even rate of 100ms.
        //
        // Otherwise, blink the LED ON for a very short time.
        if (sAppTask.mFunction != kFunction_FactoryReset)
        {
            // Consider the system to be "fully connected" if it has service
            // connectivity
            if (sHaveServiceConnectivity)
            {
                sStatusLED.Set(true);
            }
            else if (sIsWiFiStationEnabled && sIsWiFiStationProvisioned && (!sIsWiFiStationConnected || !sHaveServiceConnectivity))
            {
                sStatusLED.Blink(950, 50);
            }
            else if (sHaveBLEConnections)
            {
                sStatusLED.Blink(100, 100);
            }
            else
            {
                sStatusLED.Blink(50, 950);
            }
        }
        sStatusLED.Animate();
        sLockLED.Animate();
    }
}

void AppTask::LockActionEventHandler(AppEvent * aEvent)
{
    bool initiated = false;
    BoltLockManager::Action_t action;
    int32_t actor;
    int err = CHIP_NO_ERROR;

    if (aEvent->Type == AppEvent::kEventType_Lock)
    {
        action = static_cast<BoltLockManager::Action_t>(aEvent->LockEvent.Action);
        actor  = aEvent->LockEvent.Actor;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        if (BoltLockMgr().IsUnlocked())
        {
            action = BoltLockManager::LOCK_ACTION;
        }
        else
        {
            action = BoltLockManager::UNLOCK_ACTION;
        }
        actor = AppEvent::kEventType_Button;
    }
    else
    {
        err = CHIP_ERROR_SENTINEL;
    }

    if (err == CHIP_NO_ERROR)
    {
        initiated = BoltLockMgr().InitiateAction(actor, action);

        if (!initiated)
        {
            P6_LOG("Action is already in progress or active.");
        }
    }
}

void AppTask::ButtonEventHandler(uint8_t btnIdx, uint8_t btnAction)
{
    if (btnIdx != APP_LOCK_BUTTON_IDX && btnIdx != APP_FUNCTION_BUTTON_IDX)
    {
        return;
    }

    AppEvent button_event              = {};
    button_event.Type                  = AppEvent::kEventType_Button;
    button_event.ButtonEvent.ButtonIdx = btnIdx;
    button_event.ButtonEvent.Action    = btnAction;

    if (btnIdx == APP_LOCK_BUTTON_IDX)
    {
        button_event.Handler = LockActionEventHandler;
        sAppTask.PostEvent(&button_event);
    }
    else if (btnIdx == APP_FUNCTION_BUTTON_IDX)
    {
        button_event.Handler = FunctionHandler;
        sAppTask.PostEvent(&button_event);
    }
}

void AppTask::TimerEventHandler(TimerHandle_t xTimer)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = (void *) xTimer;
    event.Handler            = FunctionTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::FunctionTimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Timer)
    {
        return;
    }

    // If we reached here, the button was held past FACTORY_RESET_TRIGGER_TIMEOUT,
    // initiate factory reset
    if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_StartBleAdv)
    {
        P6_LOG("Factory Reset Triggered. Release button within %ums to cancel.", FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

        // Start timer for FACTORY_RESET_CANCEL_WINDOW_TIMEOUT to allow user to
        // cancel, if required.
        sAppTask.StartTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

        sAppTask.mFunction = kFunction_FactoryReset;

        // Turn off all LEDs before starting blink to make sure blink is
        // co-ordinated.
        sStatusLED.Set(false);
        sLockLED.Set(false);

        sStatusLED.Blink(500);
        sLockLED.Blink(500);
    }
    else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
    {
        // Actually trigger Factory Reset
        sAppTask.mFunction = kFunction_NoneSelected;
        ConfigurationMgr().InitiateFactoryReset();
    }
}

void AppTask::FunctionHandler(AppEvent * aEvent)
{
    if (aEvent->ButtonEvent.ButtonIdx != APP_FUNCTION_BUTTON_IDX)
    {
        return;
    }
    // To trigger software update: press the APP_FUNCTION_BUTTON button briefly (<
    // FACTORY_RESET_TRIGGER_TIMEOUT) To initiate factory reset: press the
    // APP_FUNCTION_BUTTON for FACTORY_RESET_TRIGGER_TIMEOUT +
    // FACTORY_RESET_CANCEL_WINDOW_TIMEOUT All LEDs start blinking after
    // FACTORY_RESET_TRIGGER_TIMEOUT to signal factory reset has been initiated.
    // To cancel factory reset: release the APP_FUNCTION_BUTTON once all LEDs
    // start blinking within the FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    if (aEvent->ButtonEvent.Action == APP_BUTTON_PRESSED)
    {
        if (!sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_NoneSelected)
        {
            sAppTask.StartTimer(FACTORY_RESET_TRIGGER_TIMEOUT);
            sAppTask.mFunction = kFunction_StartBleAdv;
        }
    }
    else
    {
        // If the button was released before factory reset got initiated, start Thread Network
        if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_StartBleAdv)
        {
            sAppTask.CancelTimer();
            sAppTask.mFunction = kFunction_NoneSelected;
        }
        else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
        {
            // Set lock status LED back to show state of lock.
            sLockLED.Set(!BoltLockMgr().IsUnlocked());

            sAppTask.CancelTimer();

            // Change the function to none selected since factory reset has been
            // canceled.
            sAppTask.mFunction = kFunction_NoneSelected;

            P6_LOG("Factory Reset has been Canceled");
        }
    }
}

void AppTask::CancelTimer()
{
    if (xTimerStop(sFunctionTimer, 0) == pdFAIL)
    {
        P6_LOG("app timer stop() failed");
        appError(CHIP_ERROR_SENTINEL);
    }

    mFunctionTimerActive = false;
}

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    if (xTimerIsTimerActive(sFunctionTimer))
    {
        P6_LOG("app timer already started!");
        CancelTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(sFunctionTimer, aTimeoutInMs / portTICK_PERIOD_MS, 100) != pdPASS)
    {
        P6_LOG("app timer start() failed");
        appError(CHIP_ERROR_SENTINEL);
    }

    mFunctionTimerActive = true;
}

void AppTask::ActionInitiated(BoltLockManager::Action_t aAction, int32_t aActor)
{
    // If the action has been initiated by the lock, update the bolt lock trait
    // and start flashing the LEDs rapidly to indicate action initiation.
    if (aAction == BoltLockManager::LOCK_ACTION)
    {
        P6_LOG("Lock Action has been initiated");
    }
    else if (aAction == BoltLockManager::UNLOCK_ACTION)
    {
        P6_LOG("Unlock Action has been initiated");
    }

    if (aActor == AppEvent::kEventType_Button)
    {
        sAppTask.mSyncClusterToButtonAction = true;
    }

    sLockLED.Blink(50, 50);
}

void AppTask::ActionCompleted(BoltLockManager::Action_t aAction)
{
    // if the action has been completed by the lock, update the bolt lock trait.
    // Turn on the lock LED if in a LOCKED state OR
    // Turn off the lock LED if in an UNLOCKED state.
    if (aAction == BoltLockManager::LOCK_ACTION)
    {
        P6_LOG("Lock Action has been completed");

        sLockLED.Set(true);
    }
    else if (aAction == BoltLockManager::UNLOCK_ACTION)
    {
        P6_LOG("Unlock Action has been completed");

        sLockLED.Set(false);
    }

    if (sAppTask.mSyncClusterToButtonAction)
    {
        sAppTask.UpdateClusterState();
        sAppTask.mSyncClusterToButtonAction = false;
    }
}

void AppTask::PostLockActionRequest(int32_t aActor, BoltLockManager::Action_t aAction)
{
    AppEvent event;
    event.Type             = AppEvent::kEventType_Lock;
    event.LockEvent.Actor  = aActor;
    event.LockEvent.Action = aAction;
    event.Handler          = LockActionEventHandler;
    PostEvent(&event);
}

void AppTask::PostEvent(const AppEvent * aEvent)
{
    if (sAppEventQueue != NULL)
    {
        if (!xQueueSend(sAppEventQueue, aEvent, 1))
        {
            P6_LOG("Failed to post event to app task event queue");
        }
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
        P6_LOG("Event received with no handler. Dropping event.");
    }
}

void AppTask::UpdateClusterState(void)
{
    uint8_t newValue = !BoltLockMgr().IsUnlocked();

    // write the new on/off value
    EmberAfStatus status = emberAfWriteAttribute(1, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, CLUSTER_MASK_SERVER, &newValue,
                                                 ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        P6_LOG("ERR: updating on/off %x", status);
    }
}
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char * pcTaskName)
{
    (void) pxTask;

    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */
    printf("ERROR: stack overflow with task %s\r\n", pcTaskName);
}
