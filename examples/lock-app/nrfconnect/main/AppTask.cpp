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

#include "AppTask.h"
#include "BoltLockManager.h"
#include "LEDWidget.h"
#include "Server.h"
#include "app_config.h"

#include <platform/CHIPDeviceLayer.h>

#include <dk_buttons_and_leds.h>
#include <logging/log.h>
#include <zephyr.h>

#define FACTORY_RESET_TRIGGER_TIMEOUT 3000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#define APP_EVENT_QUEUE_SIZE 10
#define BUTTON_PUSH_EVENT 1
#define BUTTON_RELEASE_EVENT 0

LOG_MODULE_DECLARE(app);
K_MSGQ_DEFINE(sAppEventQueue, sizeof(AppEvent), APP_EVENT_QUEUE_SIZE, alignof(AppEvent));

static LEDWidget sStatusLED;
static LEDWidget sLockLED;
static LEDWidget sUnusedLED;
static LEDWidget sUnusedLED_1;

static bool sIsThreadProvisioned     = false;
static bool sIsThreadEnabled         = false;
static bool sIsThreadAttached        = false;
static bool sIsPairedToAccount       = false;
static bool sHaveBLEConnections      = false;
static bool sHaveServiceConnectivity = false;

static k_timer sFunctionTimer;

using namespace ::chip::DeviceLayer;

AppTask AppTask::sAppTask;
DemoSessionManager sSessions;

int AppTask::Init()
{
    // Initialize LEDs
    LEDWidget::InitGpio();

    sStatusLED.Init(SYSTEM_STATE_LED);
    sLockLED.Init(LOCK_STATE_LED);
    sLockLED.Set(!BoltLockMgr().IsUnlocked());

    sUnusedLED.Init(DK_LED3);
    sUnusedLED_1.Init(DK_LED4);

    // Initialize buttons
    int ret = dk_buttons_init(ButtonEventHandler);
    if (ret)
    {
        LOG_ERR("dk_buttons_init() failed");
        return ret;
    }

    // Initialize timer user data
    k_timer_init(&sFunctionTimer, &AppTask::TimerEventHandler, nullptr);
    k_timer_user_data_set(&sFunctionTimer, this);

    BoltLockMgr().Init();
    BoltLockMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    // Init ZCL Data Model
    InitDataModelHandler();
    StartServer(&sSessions);
    return 0;
}

int AppTask::StartApp()
{
    int ret = Init();

    if (ret)
    {
        LOG_ERR("AppTask.Init() failed");
        return ret;
    }

    AppEvent event = {};

    while (true)
    {
        ret = k_msgq_get(&sAppEventQueue, &event, K_MSEC(10));

        while (!ret)
        {
            DispatchEvent(&event);
            ret = k_msgq_get(&sAppEventQueue, &event, K_NO_WAIT);
        }

        // Collect connectivity and configuration state from the CHIP stack.  Because the
        // CHIP event loop is being run in a separate task, the stack must be locked
        // while these values are queried.  However we use a non-blocking lock request
        // (TryLockChipStack()) to avoid blocking other UI activities when the CHIP
        // task is busy (e.g. with a long crypto operation).

        if (PlatformMgr().TryLockChipStack())
        {
            sIsThreadProvisioned = ConnectivityMgr().IsThreadProvisioned();
            sIsThreadEnabled     = ConnectivityMgr().IsThreadEnabled();
            sIsThreadAttached    = ConnectivityMgr().IsThreadAttached();
            // sHaveBLEConnections      = (ConnectivityMgr().NumBLEConnections() != 0);
            // sHaveServiceConnectivity = ConnectivityMgr().HaveServiceConnectivity();
            PlatformMgr().UnlockChipStack();
        }

        // Consider the system to be "fully connected" if it has service
        // connectivity and it is able to interact with the service on a regular basis.
        bool isFullyConnected = sHaveServiceConnectivity;

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
            if (isFullyConnected)
            {
                sStatusLED.Set(true);
            }
            else if (sIsThreadProvisioned && sIsThreadEnabled && sIsPairedToAccount && (!sIsThreadAttached || !isFullyConnected))
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
        sUnusedLED.Animate();
        sUnusedLED_1.Animate();
    }
}

void AppTask::LockActionEventHandler(AppEvent * aEvent)
{
    BoltLockManager::Action_t action = BoltLockManager::INVALID_ACTION;
    int32_t actor                    = 0;

    if (aEvent->Type == AppEvent::kEventType_Lock)
    {
        action = static_cast<BoltLockManager::Action_t>(aEvent->LockEvent.Action);
        actor  = aEvent->LockEvent.Actor;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        action = BoltLockMgr().IsUnlocked() ? BoltLockManager::LOCK_ACTION : BoltLockManager::UNLOCK_ACTION;
    }

    if (action != BoltLockManager::INVALID_ACTION && !BoltLockMgr().InitiateAction(actor, action))
        LOG_INF("Action is already in progress or active.");
}

void AppTask::ButtonEventHandler(uint32_t button_state, uint32_t has_changed)
{
    AppEvent button_event;
    button_event.Type = AppEvent::kEventType_Button;

    if (LOCK_BUTTON_MASK & button_state & has_changed)
    {
        button_event.ButtonEvent.PinNo  = LOCK_BUTTON;
        button_event.ButtonEvent.Action = BUTTON_PUSH_EVENT;
        button_event.Handler            = LockActionEventHandler;
        sAppTask.PostEvent(&button_event);
    }

    if (FUNCTION_BUTTON_MASK & has_changed)
    {
        button_event.ButtonEvent.PinNo  = FUNCTION_BUTTON;
        button_event.ButtonEvent.Action = (FUNCTION_BUTTON_MASK & button_state) ? BUTTON_PUSH_EVENT : BUTTON_RELEASE_EVENT;
        button_event.Handler            = FunctionHandler;
        sAppTask.PostEvent(&button_event);
    }
}

void AppTask::TimerEventHandler(k_timer * timer)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = k_timer_user_data_get(timer);
    event.Handler            = FunctionTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::FunctionTimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Timer)
        return;

    // If we reached here, the button was held past FACTORY_RESET_TRIGGER_TIMEOUT, initiate factory reset
    if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_SoftwareUpdate)
    {
        LOG_INF("Factory Reset Triggered. Release button within %ums to cancel.", FACTORY_RESET_TRIGGER_TIMEOUT);

        // Start timer for FACTORY_RESET_CANCEL_WINDOW_TIMEOUT to allow user to cancel, if required.
        sAppTask.StartTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);
        sAppTask.mFunction = kFunction_FactoryReset;

        // Turn off all LEDs before starting blink to make sure blink is co-ordinated.
        sStatusLED.Set(false);
        sLockLED.Set(false);
        sUnusedLED_1.Set(false);
        sUnusedLED.Set(false);

        sStatusLED.Blink(500);
        sLockLED.Blink(500);
        sUnusedLED.Blink(500);
        sUnusedLED_1.Blink(500);
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
    if (aEvent->ButtonEvent.PinNo != FUNCTION_BUTTON)
        return;

    // To trigger software update: press the FUNCTION_BUTTON button briefly (< FACTORY_RESET_TRIGGER_TIMEOUT)
    // To initiate factory reset: press the FUNCTION_BUTTON for FACTORY_RESET_TRIGGER_TIMEOUT + FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    // All LEDs start blinking after FACTORY_RESET_TRIGGER_TIMEOUT to signal factory reset has been initiated.
    // To cancel factory reset: release the FUNCTION_BUTTON once all LEDs start blinking within the
    // FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    if (aEvent->ButtonEvent.Action == BUTTON_PUSH_EVENT)
    {
        if (!sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_NoneSelected)
        {
            sAppTask.StartTimer(FACTORY_RESET_TRIGGER_TIMEOUT);

            sAppTask.mFunction = kFunction_SoftwareUpdate;
        }
    }
    else
    {
        // If the button was released before factory reset got initiated, trigger a software update.
        if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_SoftwareUpdate)
        {
            sAppTask.CancelTimer();
            sAppTask.mFunction = kFunction_NoneSelected;
            LOG_INF("Software update is not implemented");
        }
        else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
        {
            sUnusedLED.Set(false);
            sUnusedLED_1.Set(false);

            // Set lock status LED back to show state of lock.
            sLockLED.Set(!BoltLockMgr().IsUnlocked());

            sAppTask.CancelTimer();

            // Change the function to none selected since factory reset has been canceled.
            sAppTask.mFunction = kFunction_NoneSelected;

            LOG_INF("Factory Reset has been Canceled");
        }
    }
}

void AppTask::CancelTimer()
{
    k_timer_stop(&sFunctionTimer);
    mFunctionTimerActive = false;
}

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    k_timer_start(&sFunctionTimer, K_MSEC(aTimeoutInMs), K_NO_WAIT);
    mFunctionTimerActive = true;
}

void AppTask::ActionInitiated(BoltLockManager::Action_t aAction, int32_t aActor)
{
    // If the action has been initiated by the lock, update the bolt lock trait
    // and start flashing the LEDs rapidly to indicate action initiation.
    if (aAction == BoltLockManager::LOCK_ACTION)
    {
        LOG_INF("Lock Action has been initiated");
    }
    else if (aAction == BoltLockManager::UNLOCK_ACTION)
    {
        LOG_INF("Unlock Action has been initiated");
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
        LOG_INF("Lock Action has been completed");
        sLockLED.Set(true);
    }
    else if (aAction == BoltLockManager::UNLOCK_ACTION)
    {
        LOG_INF("Unlock Action has been completed");
        sLockLED.Set(false);
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

void AppTask::PostEvent(AppEvent * aEvent)
{
    if (k_msgq_put(&sAppEventQueue, aEvent, K_TICKS(1)))
    {
        LOG_INF("Failed to post event to app task event queue");
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
        LOG_INF("Event received with no handler. Dropping event.");
    }
}
