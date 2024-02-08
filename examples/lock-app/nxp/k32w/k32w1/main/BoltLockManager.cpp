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

#include "BoltLockManager.h"

#include "AppTask.h"
#include "FreeRTOS.h"

#include "app_config.h"

BoltLockManager BoltLockManager::sLock;

TimerHandle_t sLockTimer; // FreeRTOS app sw timer.

int BoltLockManager::Init()
{
    int err = 0;

    // Create FreeRTOS sw timer for Lock timer.

    sLockTimer = xTimerCreate("LockTmr",        // Just a text name, not used by the RTOS kernel
                              1,                // == default timer period (mS)
                              false,            // no timer reload (==one-shot)
                              (void *) this,    // init timer id = lock obj context
                              TimerEventHandler // timer callback handler
    );

    if (sLockTimer == NULL)
    {
        K32W_LOG("lock timer create failed");
        assert(0);
    }

    mState              = kState_LockingCompleted;
    mAutoLockTimerArmed = false;
    mAutoRelock         = false;
    mAutoLockDuration   = 0;

    return err;
}

void BoltLockManager::SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

bool BoltLockManager::IsActionInProgress()
{
    return (mState == kState_LockingInitiated || mState == kState_UnlockingInitiated) ? true : false;
}

bool BoltLockManager::IsUnlocked()
{
    return (mState == kState_UnlockingCompleted) ? true : false;
}

void BoltLockManager::EnableAutoRelock(bool aOn)
{
    mAutoRelock = aOn;
}

void BoltLockManager::SetAutoLockDuration(uint32_t aDurationInSecs)
{
    mAutoLockDuration = aDurationInSecs;
}

bool BoltLockManager::InitiateAction(int32_t aActor, Action_t aAction)
{
    bool action_initiated = false;
    State_t new_state;

    // Initiate Lock/Unlock Action only when the previous one is complete.
    if (mState == kState_LockingCompleted && aAction == UNLOCK_ACTION)
    {
        action_initiated = true;

        new_state = kState_UnlockingInitiated;
    }
    else if (mState == kState_UnlockingCompleted && aAction == LOCK_ACTION)
    {
        action_initiated = true;

        new_state = kState_LockingInitiated;
    }

    if (action_initiated)
    {
        if (mAutoLockTimerArmed && new_state == kState_LockingInitiated)
        {
            // If auto lock timer has been armed and someone initiates locking,
            // cancel the timer and continue as normal.
            mAutoLockTimerArmed = false;

            CancelTimer();
        }

        StartTimer(ACTUATOR_MOVEMENT_PERIOS_MS);

        // Since the timer started successfully, update the state and trigger callback
        mState = new_state;

        if (mActionInitiated_CB)
        {
            mActionInitiated_CB(aAction, aActor);
        }
    }

    return action_initiated;
}

void BoltLockManager::StartTimer(uint32_t aTimeoutMs)
{
    if (xTimerIsTimerActive(sLockTimer))
    {
        K32W_LOG("lock timer already started!");
        // appError(err);
        CancelTimer();
    }

    // timer is not active, change its period to required value.
    // This also causes the timer to start.  FreeRTOS- Block for a maximum of
    // 100 ticks if the  change period command cannot immediately be sent to the
    // timer command queue.
    if (xTimerChangePeriod(sLockTimer, aTimeoutMs / portTICK_PERIOD_MS, 100) != pdPASS)
    {
        K32W_LOG("lock timer start() failed");
        // appError(err);
    }
}

void BoltLockManager::CancelTimer(void)
{
    if (xTimerStop(sLockTimer, 0) == pdFAIL)
    {
        K32W_LOG("lock timer stop() failed");
        // appError(err);
    }
}

void BoltLockManager::TimerEventHandler(TimerHandle_t xTimer)
{
    // Get lock obj context from timer id.
    BoltLockManager * lock = static_cast<BoltLockManager *>(pvTimerGetTimerID(xTimer));

    // The timer event handler will be called in the context of the timer task
    // once sLockTimer expires. Post an event to apptask queue with the actual handler
    // so that the event can be handled in the context of the apptask.
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = lock;

    if (lock->mAutoLockTimerArmed)
    {
        event.Handler = AutoReLockTimerEventHandler;
        GetAppTask().PostEvent(&event);
    }
    else
    {
        event.Handler = ActuatorMovementTimerEventHandler;
        GetAppTask().PostEvent(&event);
    }
}

void BoltLockManager::AutoReLockTimerEventHandler(void * aGenericEvent)
{
    AppEvent * aEvent      = (AppEvent *) aGenericEvent;
    BoltLockManager * lock = static_cast<BoltLockManager *>(aEvent->TimerEvent.Context);
    int32_t actor          = 0;

    // Make sure auto lock timer is still armed.
    if (!lock->mAutoLockTimerArmed)
    {
        return;
    }

    lock->mAutoLockTimerArmed = false;

    K32W_LOG("Auto Re-Lock has been triggered!");

    lock->InitiateAction(actor, LOCK_ACTION);
}

void BoltLockManager::ActuatorMovementTimerEventHandler(void * aGenericEvent)
{
    AppEvent * aEvent        = (AppEvent *) aGenericEvent;
    Action_t actionCompleted = INVALID_ACTION;

    BoltLockManager * lock = static_cast<BoltLockManager *>(aEvent->TimerEvent.Context);

    if (lock->mState == kState_LockingInitiated)
    {
        lock->mState    = kState_LockingCompleted;
        actionCompleted = LOCK_ACTION;
    }
    else if (lock->mState == kState_UnlockingInitiated)
    {
        lock->mState    = kState_UnlockingCompleted;
        actionCompleted = UNLOCK_ACTION;
    }

    if (actionCompleted != INVALID_ACTION)
    {
        if (lock->mActionCompleted_CB)
        {
            lock->mActionCompleted_CB(actionCompleted);
        }

        if (lock->mAutoRelock && actionCompleted == UNLOCK_ACTION)
        {
            // Start the timer for auto relock
            lock->StartTimer(lock->mAutoLockDuration * 1000);

            lock->mAutoLockTimerArmed = true;

            K32W_LOG("Auto Re-lock enabled. Will be triggered in %u seconds", lock->mAutoLockDuration);
        }
    }
}
