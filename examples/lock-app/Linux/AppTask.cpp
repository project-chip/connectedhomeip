/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#define FACTORY_RESET_TRIGGER_TIMEOUT 3000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#define APP_TASK_STACK_SIZE 4096
#define APP_TASK_PRIORITY 2
#define APP_EVENT_QUEUE_SIZE 16

#define LOCK_BUTTON 1
#define FUNCTION_BUTTON 1
#define APP_BUTTON_PUSH 1

static struct chip_os_timer sFunctionTimer;
static struct chip_os_mutex sCHIPEventLock;

static struct chip_os_task sAppTaskHandle;
static struct chip_os_queue sAppEventQueue;

static bool sIsThreadProvisioned     = false;
static bool sIsThreadEnabled         = false;
static bool sIsThreadAttached        = false;
static bool sHaveBLEConnections      = false;
static bool sHaveServiceConnectivity = false;

using namespace ::chip::DeviceLayer;

AppTask AppTask::sAppTask;

int AppTask::StartAppTask()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    chip_os_queue_init(&sAppEventQueue, sizeof(AppEvent), APP_EVENT_QUEUE_SIZE);
    if (!chip_os_queue_inited(&sAppEventQueue))
    {
        APP_LOG("Failed to allocate app event queue");
        err = CHIP_ERROR_INCORRECT_STATE;
        ;
        assert(true);
    }

    // Start App task.
    if (chip_os_task_init(&sAppTaskHandle, "APP", AppTaskMain, NULL, APP_TASK_PRIORITY, APP_TASK_STACK_SIZE))
    {
        err = CHIP_ERROR_INCORRECT_STATE;
    }

    return err;
}

int AppTask::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = BoltLockMgr().Init();
    if (err)
    {
        APP_LOG("BoltLockMgr().Init() failed");
        assert(true);
    }

    if (chip_os_mutex_init(&sCHIPEventLock))
    {
        APP_LOG("AppTask::Init mutex init failed");
        assert(true);
    }

    chip_os_timer_init(&sFunctionTimer, TimerEventHandler, (void *) this);

    BoltLockMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    return err;
}

void * AppTask::AppTaskMain(void * pvParameter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AppEvent event;

    APP_LOG("AppTask started");

    err = sAppTask.Init();
    if (err)
    {
        APP_LOG("AppTask.Init() failed: %d", err);
        assert(true);
    }

    while (true)
    {
        chip_os_error_t eventError = chip_os_queue_get(&sAppEventQueue, &event, CHIP_OS_TIME_FOREVER);

        while (eventError == CHIP_OS_OK)
        {
            sAppTask.DispatchEvent(&event);

            eventError = chip_os_queue_get(&sAppEventQueue, &event, 0);
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
            sIsThreadAttached        = ConnectivityMgr().IsThreadAttached();
            sHaveBLEConnections      = (ConnectivityMgr().NumBLEConnections() != 0);
            sHaveServiceConnectivity = ConnectivityMgr().HaveServiceConnectivity();
            PlatformMgr().UnlockChipStack();
        }
    }

    return NULL;
}

void AppTask::LockActionEventHandler(AppEvent * aEvent)
{
    bool initiated = false;
    BoltLockManager::Action_t action;
    int32_t actor  = 0;
    CHIP_ERROR err = CHIP_NO_ERROR;

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
    }
    else
    {
        err = CHIP_ERROR_INCORRECT_STATE;
    }

    if (err == CHIP_NO_ERROR)
    {
        initiated = BoltLockMgr().InitiateAction(actor, action);

        if (!initiated)
        {
            APP_LOG("Action is already in progress or active.");
        }
    }
}

void AppTask::ButtonEventHandler(uint8_t pin_no, uint8_t button_action)
{
    if (pin_no != LOCK_BUTTON && pin_no != FUNCTION_BUTTON)
    {
        return;
    }

    AppEvent button_event;
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.PinNo  = pin_no;
    button_event.ButtonEvent.Action = button_action;

    if (pin_no == LOCK_BUTTON && button_action == APP_BUTTON_PUSH)
    {
        button_event.Handler = LockActionEventHandler;
    }
    else if (pin_no == FUNCTION_BUTTON)
    {
        button_event.Handler = FunctionHandler;
    }

    sAppTask.PostEvent(&button_event);
}

void AppTask::TimerEventHandler(void * p_context)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = p_context;
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
        APP_LOG("Factory Reset Triggered. Release button within %ums to cancel.", FACTORY_RESET_TRIGGER_TIMEOUT);

        // Start timer for FACTORY_RESET_CANCEL_WINDOW_TIMEOUT to allow user to cancel, if required.
        sAppTask.StartTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

        sAppTask.mFunction = kFunction_FactoryReset;
    }
    else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
    {
        // Actually trigger Factory Reset
        sAppTask.mFunction = kFunction_NoneSelected;
        APP_LOG("Factory reset is not implemented");
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
    if (aEvent->ButtonEvent.Action == APP_BUTTON_PUSH)
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
            APP_LOG("Software update is not implemented");
        }
        else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
        {
            sAppTask.CancelTimer();

            // Change the function to none selected since factory reset has been canceled.
            sAppTask.mFunction = kFunction_NoneSelected;

            APP_LOG("Factory Reset has been Canceled");
        }
    }
}

void AppTask::CancelTimer()
{
    chip_os_timer_stop(&sFunctionTimer);
    mFunctionTimerActive = false;
}

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    if (chip_os_timer_is_active(&sFunctionTimer))
    {
        APP_LOG("app timer already started!");
        CancelTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    chip_os_timer_start(&sFunctionTimer, aTimeoutInMs);

    mFunctionTimerActive = true;
}

void AppTask::ActionInitiated(BoltLockManager::Action_t aAction, int32_t aActor)
{
    // If the action has been initiated by the lock, update the bolt lock trait
    // and start flashing the LEDs rapidly to indicate action initiation.
    if (aAction == BoltLockManager::LOCK_ACTION)
    {
        APP_LOG("Lock Action has been initiated");
    }
    else if (aAction == BoltLockManager::UNLOCK_ACTION)
    {
        APP_LOG("Unlock Action has been initiated");
    }
}

void AppTask::ActionCompleted(BoltLockManager::Action_t aAction)
{
    // if the action has been completed by the lock, update the bolt lock trait.
    // Turn on the lock LED if in a LOCKED state OR
    // Turn off the lock LED if in an UNLOCKED state.
    if (aAction == BoltLockManager::LOCK_ACTION)
    {
        APP_LOG("Lock Action has been completed");
    }
    else if (aAction == BoltLockManager::UNLOCK_ACTION)
    {
        APP_LOG("Unlock Action has been completed");
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
    if (chip_os_queue_inited(&sAppEventQueue))
    {
        if (chip_os_queue_put(&sAppEventQueue, (void *) aEvent))
        {
            APP_LOG("Failed to post event to app task event queue");
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
        APP_LOG("Event received with no handler. Dropping event.");
    }
}
