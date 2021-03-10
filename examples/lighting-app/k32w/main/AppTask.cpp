/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2021 Google LLC.
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
#include "AppEvent.h"
#include "Server.h"
#include "support/ErrorStr.h"

#include "QRCodeUtil.h"
#include <platform/CHIPDeviceLayer.h>
#include <platform/internal/DeviceNetworkInfo.h>

#include "attribute-storage.h"
#include "gen/attribute-id.h"
#include "gen/attribute-type.h"
#include "gen/cluster-id.h"

#include "Keyboard.h"
#include "LED.h"
#include "LEDWidget.h"
#include "TimersManager.h"
#include "app_config.h"

#define FACTORY_RESET_TRIGGER_TIMEOUT 6000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#define APP_TASK_STACK_SIZE (4096)
#define APP_TASK_PRIORITY 2
#define APP_EVENT_QUEUE_SIZE 10

TimerHandle_t sFunctionTimer; // FreeRTOS app sw timer.

static QueueHandle_t sAppEventQueue;

static LEDWidget sStatusLED;
static LEDWidget sLightLED;

static bool sIsThreadProvisioned     = false;
static bool sIsThreadEnabled         = false;
static bool sHaveBLEConnections      = false;
static bool sHaveServiceConnectivity = false;

static uint32_t eventMask = 0;

using namespace ::chip::DeviceLayer;

AppTask AppTask::sAppTask;

int AppTask::StartAppTask()
{
    int err = CHIP_NO_ERROR;

    sAppEventQueue = xQueueCreate(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent));
    if (sAppEventQueue == NULL)
    {
        err = CHIP_ERROR_MAX;
        K32W_LOG("Failed to allocate app event queue");
        assert(err == CHIP_NO_ERROR);
    }

    return err;
}

int AppTask::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Init ZCL Data Model and start server
    InitServer();

    // QR code will be used with CHIP Tool
    PrintQRCode(chip::RendezvousInformationFlags::kBLE);

    TMR_Init();

    /* HW init leds */
    LED_Init();

    /* start with all LEDS turnedd off */
    sStatusLED.Init(SYSTEM_STATE_LED);

    sLightLED.Init(LIGHT_STATE_LED);
    sLightLED.Set(LightingMgr().IsTurnedOff());

    /* intialize the Keyboard and button press calback */
    KBD_Init(KBD_Callback);

    // Create FreeRTOS sw timer for Function Selection.
    sFunctionTimer = xTimerCreate("FnTmr",          // Just a text name, not used by the RTOS kernel
                                  1,                // == default timer period (mS)
                                  false,            // no timer reload (==one-shot)
                                  (void *) this,    // init timer id = app task obj context
                                  TimerEventHandler // timer callback handler
    );
    if (sFunctionTimer == NULL)
    {
        K32W_LOG("app_timer_create() failed");
        assert(err == CHIP_NO_ERROR);
    }

    err = LightingMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        K32W_LOG("LightingMgr().Init() failed");
        assert(err == CHIP_NO_ERROR);
    }

    LightingMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    // Print the current software version
    char currentFirmwareRev[ConfigurationManager::kMaxFirmwareRevisionLength + 1] = { 0 };
    size_t currentFirmwareRevLen;
    err = ConfigurationMgr().GetFirmwareRevision(currentFirmwareRev, sizeof(currentFirmwareRev), currentFirmwareRevLen);
    if (err != CHIP_NO_ERROR)
    {
        K32W_LOG("Get version error");
        assert(err == CHIP_NO_ERROR);
    }

    K32W_LOG("Current Firmware Version: %s", currentFirmwareRev);

    return err;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    int err;
    AppEvent event;

    err = sAppTask.Init();
    if (err != CHIP_NO_ERROR)
    {
        K32W_LOG("AppTask.Init() failed");
        assert(err == CHIP_NO_ERROR);
    }

    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, pdMS_TO_TICKS(10));
        while (eventReceived == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
            eventReceived = xQueueReceive(sAppEventQueue, &event, 0);
        }

        // Collect connectivity and configuration state from the CHIP stack.  Because the
        // CHIP event loop is being run in a separate task, the stack must be locked
        // while these values are queried.  However we use a non-blocking lock request
        // (TryLockChipStack()) to avoid blocking other UI activities when the CHIP
        // task is busy (e.g. with a long crypto operation).
        if (PlatformMgr().TryLockChipStack())
        {
            sIsThreadProvisioned     = ConnectivityMgr().IsThreadProvisioned();
            sIsThreadEnabled         = ConnectivityMgr().IsThreadEnabled();
            sHaveBLEConnections      = (ConnectivityMgr().NumBLEConnections() != 0);
            sHaveServiceConnectivity = ConnectivityMgr().HaveServiceConnectivity();
            PlatformMgr().UnlockChipStack();
        }

        // Update the status LED if factory reset has not been initiated.
        //
        // If system has "full connectivity", keep the LED On constantly.
        //
        // If thread and service provisioned, but not attached to the thread network yet OR no
        // connectivity to the service OR subscriptions are not fully established
        // THEN blink the LED Off for a short period of time.
        //
        // If the system has ble connection(s) uptill the stage above, THEN blink the LEDs at an even
        // rate of 100ms.
        //
        // Otherwise, blink the LED ON for a very short time.
        if (sAppTask.mFunction != kFunction_FactoryReset)
        {
            if (sHaveServiceConnectivity)
            {
                sStatusLED.Set(true);
            }
            else if (sIsThreadProvisioned && sIsThreadEnabled)
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
        sLightLED.Animate();

        HandleKeyboard();
    }
}

void AppTask::ButtonEventHandler(uint8_t pin_no, uint8_t button_action)
{
    if ((pin_no != RESET_BUTTON) && (pin_no != LIGHT_BUTTON) && (pin_no != JOIN_BUTTON))
    {
        return;
    }

    AppEvent button_event;
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.PinNo  = pin_no;
    button_event.ButtonEvent.Action = button_action;

    if (pin_no == RESET_BUTTON)
    {
        button_event.Handler = ResetActionEventHandler;
    }
    else if (pin_no == LIGHT_BUTTON)
    {
        button_event.Handler = LightActionEventHandler;
    }
    else if (pin_no == JOIN_BUTTON)
    {
        button_event.Handler = JoinHandler;
    }

    sAppTask.PostEvent(&button_event);
}

void AppTask::KBD_Callback(uint8_t events)
{
    eventMask = eventMask | (uint32_t)(1 << events);
}

void AppTask::HandleKeyboard(void)
{
    uint8_t keyEvent = 0xFF;
    uint8_t pos      = 0;

    while (eventMask)
    {
        for (pos = 0; pos < (8 * sizeof(eventMask)); pos++)
        {
            if (eventMask & (1 << pos))
            {
                keyEvent  = pos;
                eventMask = eventMask & ~(1 << pos);
                break;
            }
        }

        switch (keyEvent)
        {
        case gKBD_EventPB1_c:
            ButtonEventHandler(RESET_BUTTON, RESET_BUTTON_PUSH);
            break;
        case gKBD_EventPB2_c:
            ButtonEventHandler(LIGHT_BUTTON, LIGHT_BUTTON_PUSH);
            break;
        case gKBD_EventPB3_c:
            ButtonEventHandler(JOIN_BUTTON, JOIN_BUTTON_PUSH);
            break;
        default:
            break;
        }
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
        return;

    K32W_LOG("Device will factory reset...");

    // Actually trigger Factory Reset
    ConfigurationMgr().InitiateFactoryReset();
}

void AppTask::ResetActionEventHandler(AppEvent * aEvent)
{
    if (aEvent->ButtonEvent.PinNo != RESET_BUTTON)
        return;

    if (sAppTask.mResetTimerActive)
    {
        sAppTask.CancelTimer();
        sAppTask.mFunction = kFunction_NoneSelected;

        /* restore initial state for the LED indicating Lighting state */
        if (LightingMgr().IsTurnedOff())
        {
            sLightLED.Set(false);
        }
        else
        {
            sLightLED.Set(true);
        }

        K32W_LOG("Factory Reset was cancelled!");
    }
    else
    {
        uint32_t resetTimeout = FACTORY_RESET_TRIGGER_TIMEOUT;

        if (sAppTask.mFunction != kFunction_NoneSelected)
        {
            K32W_LOG("Another function is scheduled. Could not initiate Factory Reset!");
            return;
        }

        K32W_LOG("Factory Reset Triggered. Push the RESET button within %u ms to cancel!", resetTimeout);
        sAppTask.mFunction = kFunction_FactoryReset;

        /* LEDs will start blinking to signal that a Factory Reset was scheduled */
        sStatusLED.Set(false);
        sLightLED.Set(false);

        sStatusLED.Blink(500);
        sLightLED.Blink(500);

        sAppTask.StartTimer(FACTORY_RESET_TRIGGER_TIMEOUT);
    }
}

void AppTask::LightActionEventHandler(AppEvent * aEvent)
{
    LightingManager::Action_t action;
    int err        = CHIP_NO_ERROR;
    int32_t actor  = 0;
    bool initiated = false;

    if (sAppTask.mFunction != kFunction_NoneSelected)
    {
        K32W_LOG("Another function is scheduled. Could not initiate ON/OFF Light command!");
        return;
    }

    if (aEvent->Type == AppEvent::kEventType_TurnOn)
    {
        action = static_cast<LightingManager::Action_t>(aEvent->LightEvent.Action);
        actor  = aEvent->LightEvent.Actor;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        if (LightingMgr().IsTurnedOff())
        {
            action = LightingManager::TURNON_ACTION;
        }
        else
        {
            action = LightingManager::TURNOFF_ACTION;
        }
    }
    else
    {
        err = CHIP_ERROR_MAX;
    }

    if (err == CHIP_NO_ERROR)
    {
        initiated = LightingMgr().InitiateAction(actor, action);

        if (!initiated)
        {
            K32W_LOG("Action is already in progress or active.");
        }
    }
}

void AppTask::ThreadStart()
{
    chip::DeviceLayer::Internal::DeviceNetworkInfo networkInfo;

    memset(networkInfo.ThreadNetworkName, 0, chip::DeviceLayer::Internal::kMaxThreadNetworkNameLength + 1);
    memcpy(networkInfo.ThreadNetworkName, "OpenThread", 10);

    networkInfo.ThreadExtendedPANId[0] = 0xde;
    networkInfo.ThreadExtendedPANId[1] = 0xad;
    networkInfo.ThreadExtendedPANId[2] = 0x00;
    networkInfo.ThreadExtendedPANId[3] = 0xbe;
    networkInfo.ThreadExtendedPANId[4] = 0xef;
    networkInfo.ThreadExtendedPANId[5] = 0x00;
    networkInfo.ThreadExtendedPANId[6] = 0xca;
    networkInfo.ThreadExtendedPANId[7] = 0xfe;

    networkInfo.ThreadMasterKey[0]  = 0x00;
    networkInfo.ThreadMasterKey[1]  = 0x11;
    networkInfo.ThreadMasterKey[2]  = 0x22;
    networkInfo.ThreadMasterKey[3]  = 0x33;
    networkInfo.ThreadMasterKey[4]  = 0x44;
    networkInfo.ThreadMasterKey[5]  = 0x55;
    networkInfo.ThreadMasterKey[6]  = 0x66;
    networkInfo.ThreadMasterKey[7]  = 0x77;
    networkInfo.ThreadMasterKey[8]  = 0x88;
    networkInfo.ThreadMasterKey[9]  = 0x99;
    networkInfo.ThreadMasterKey[10] = 0xAA;
    networkInfo.ThreadMasterKey[11] = 0xBB;
    networkInfo.ThreadMasterKey[12] = 0xCC;
    networkInfo.ThreadMasterKey[13] = 0xDD;
    networkInfo.ThreadMasterKey[14] = 0xEE;
    networkInfo.ThreadMasterKey[15] = 0xFF;

    networkInfo.ThreadPANId   = 0xabcd;
    networkInfo.ThreadChannel = 15;

    networkInfo.FieldPresent.ThreadExtendedPANId = true;
    networkInfo.FieldPresent.ThreadMeshPrefix    = false;
    networkInfo.FieldPresent.ThreadPSKc          = false;
    networkInfo.NetworkId                        = 0;
    networkInfo.FieldPresent.NetworkId           = true;

    ThreadStackMgr().SetThreadEnabled(false);
    ThreadStackMgr().SetThreadProvision(networkInfo);
    ThreadStackMgr().SetThreadEnabled(true);
}

void AppTask::JoinHandler(AppEvent * aEvent)
{
    if (aEvent->ButtonEvent.PinNo != JOIN_BUTTON)
        return;

    if (sAppTask.mFunction != kFunction_NoneSelected)
    {
        K32W_LOG("Another function is scheduled. Could not initiate Thread Join!");
        return;
    }

    /* hard-code Thread Commissioning Parameters for the moment.
     * In a future PR, these parameters will be sent via BLE.
     */
    ThreadStart();
}

void AppTask::CancelTimer()
{
    if (xTimerStop(sFunctionTimer, 0) == pdFAIL)
    {
        K32W_LOG("app timer stop() failed");
    }

    mResetTimerActive = false;
}

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    if (xTimerIsTimerActive(sFunctionTimer))
    {
        K32W_LOG("app timer already started!");
        CancelTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(sFunctionTimer, aTimeoutInMs / portTICK_PERIOD_MS, 100) != pdPASS)
    {
        K32W_LOG("app timer start() failed");
    }

    mResetTimerActive = true;
}

void AppTask::ActionInitiated(LightingManager::Action_t aAction, int32_t aActor)
{
    // start flashing the LEDs rapidly to indicate action initiation.
    if (aAction == LightingManager::TURNON_ACTION)
    {
        K32W_LOG("Turn on Action has been initiated")
    }
    else if (aAction == LightingManager::TURNOFF_ACTION)
    {
        K32W_LOG("Turn off Action has been initiated")
    }

    sAppTask.mFunction = kFunctionTurnOnTurnOff;
}

void AppTask::ActionCompleted(LightingManager::Action_t aAction)
{
    // Turn on the light LED if in a TURNON state OR
    // Turn off the light LED if in a TURNOFF state.
    if (aAction == LightingManager::TURNON_ACTION)
    {
        K32W_LOG("Turn on action has been completed")
        sLightLED.Set(true);
    }
    else if (aAction == LightingManager::TURNOFF_ACTION)
    {
        K32W_LOG("Turn off action has been completed")
        sLightLED.Set(false);
    }

    sAppTask.mFunction = kFunction_NoneSelected;
}

void AppTask::PostTurnOnActionRequest(int32_t aActor, LightingManager::Action_t aAction)
{
    AppEvent event;
    event.Type              = AppEvent::kEventType_TurnOn;
    event.LightEvent.Actor  = aActor;
    event.LightEvent.Action = aAction;
    event.Handler           = LightActionEventHandler;
    PostEvent(&event);
}

void AppTask::PostEvent(const AppEvent * aEvent)
{
    if (sAppEventQueue != NULL)
    {
        if (!xQueueSend(sAppEventQueue, aEvent, 1))
        {
            K32W_LOG("Failed to post event to app task event queue");
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
        K32W_LOG("Event received with no handler. Dropping event.");
    }
}

void AppTask::UpdateClusterState(void)
{
    uint8_t newValue = !LightingMgr().IsTurnedOff();

    // write the new on/off value
    EmberAfStatus status = emberAfWriteAttribute(1, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                                 (uint8_t *) &newValue, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ERR: updating on/off %x", status);
    }
}
