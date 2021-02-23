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

#include "AppConfig.h"
#include "AppTask.h"
#include "FreeRTOS.h"

#define ACTUATOR_MOVEMENT_PERIOS_MS 500

BoltLockManager BoltLockManager::sLock;

int BoltLockManager::Init()
{
    int ret = 0;

    mTimerHandle = xTimerCreate("BLT_TIMER", pdMS_TO_TICKS(ACTUATOR_MOVEMENT_PERIOS_MS), pdFALSE, this, TimerEventHandler);
    if (NULL == mTimerHandle)
    {
        PLAT_LOG("failed to create bolt lock timer");
        while (1)
            ;
    }

    mState              = kState_LockingCompleted;
    mAutoLockTimerArmed = false;
    mAutoRelock         = false;
    mAutoLockDuration   = 0;

    return ret;
}

void BoltLockManager::SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

bool BoltLockManager::IsActionInProgress()
{
    return (mState == kState_LockingInitiated || mState == kState_UnlockingInitiated);
}

bool BoltLockManager::IsUnlocked()
{
    return (mState == kState_UnlockingCompleted);
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
    xTimerChangePeriod(mTimerHandle, pdMS_TO_TICKS(aTimeoutMs), 100);
    xTimerStart(mTimerHandle, 100);
}

void BoltLockManager::CancelTimer(void)
{
    xTimerStop(mTimerHandle, 100);
}

void BoltLockManager::TimerEventHandler(TimerHandle_t aTimer)
{
    BoltLockManager * lock = static_cast<BoltLockManager *>(pvTimerGetTimerID(aTimer));

    // The timer event handler will be called in the context of the timer task
    // once sLockTimer expires. Post an event to apptask queue with the actual handler
    // so that the event can be handled in the context of the apptask.
    AppEvent event;
    event.Type                  = AppEvent::kEventType_AppEvent;
    event.BoltLockEvent.Context = static_cast<BoltLockManager *>(lock);
    if (lock->mAutoLockTimerArmed)
    {
        event.Handler = AutoReLockTimerEventHandler;
    }
    else
    {
        event.Handler = ActuatorMovementTimerEventHandler;
    }
    GetAppTask().PostEvent(&event);
}

void BoltLockManager::AutoReLockTimerEventHandler(AppEvent * aEvent)
{
    BoltLockManager * lock = static_cast<BoltLockManager *>(aEvent->BoltLockEvent.Context);
    int32_t actor          = 0;

    // Make sure auto lock timer is still armed.
    if (!lock->mAutoLockTimerArmed)
    {
        return;
    }

    lock->mAutoLockTimerArmed = false;

    PLAT_LOG("Auto Re-Lock has been triggered!");

    lock->InitiateAction(actor, LOCK_ACTION);
}

void BoltLockManager::ActuatorMovementTimerEventHandler(AppEvent * aEvent)
{
    Action_t actionCompleted = INVALID_ACTION;

    BoltLockManager * lock = static_cast<BoltLockManager *>(aEvent->BoltLockEvent.Context);

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

            PLAT_LOG("Auto Re-lock enabled. Will be triggered in %u seconds", lock->mAutoLockDuration);
        }
    }
}
