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
#include "wiced_bt_event.h"

BoltLockManager BoltLockManager::sLock;

wiced_timer_t sLockTimer;

CHIP_ERROR BoltLockManager::Init()
{
    wiced_result_t result;

    // Create wiced timer for lock timer.
    result = wiced_init_timer(&sLockTimer, TimerEventHandler, (WICED_TIMER_PARAM_TYPE) this, WICED_MILLI_SECONDS_TIMER);
    if (result != WICED_BT_SUCCESS)
    {
        printf("sLockTimer timer create failed");
        return APP_ERROR_INIT_TIMER_FAILED;
    }

    mState              = kState_UnlockingCompleted;
    mAutoLockTimerArmed = false;
    mAutoRelock         = false;
    mAutoLockDuration   = 0;

    return CHIP_NO_ERROR;
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
    if (wiced_is_timer_in_use(&sLockTimer))
    {
        printf("app timer already started!\n");
        CancelTimer();
    }

    if (wiced_start_timer(&sLockTimer, aTimeoutMs) != WICED_BT_SUCCESS)
    {
        printf("sLockTimer timer start() failed\n");
        return;
    }
}

void BoltLockManager::CancelTimer(void)
{
    if (wiced_stop_timer(&sLockTimer) != WICED_BT_SUCCESS)
    {
        printf("Lock timer timer stop() failed\n");
        return;
    }
}

void BoltLockManager::TimerEventHandler(WICED_TIMER_PARAM_TYPE cb_params)
{
    BoltLockManager * lock = reinterpret_cast<BoltLockManager *>(cb_params);
    int (*fn)(void *);

    // The timer event handler will be called in the context of the timer task
    // once sLockTimer expires. Post an event to apptask queue with the actual handler
    // so that the event can be handled in the context of the apptask.
    if (lock->mAutoLockTimerArmed)
    {
        fn = AutoReLockTimerEventHandler;
    }
    else
    {
        fn = ActuatorMovementTimerEventHandler;
    }

    if (wiced_app_event_serialize(fn, (void *) lock) != WICED_TRUE)
    {
        printf("wiced_app_event_serialize failed\n");
    }
}

int BoltLockManager::AutoReLockTimerEventHandler(void * data)
{
    BoltLockManager * lock = reinterpret_cast<BoltLockManager *>(data);
    int32_t actor          = 0;

    // Make sure auto lock timer is still armed.
    if (!lock->mAutoLockTimerArmed)
    {
        return 0;
    }

    lock->mAutoLockTimerArmed = false;

    printf("Auto Re-Lock has been triggered!\n");

    lock->InitiateAction(actor, LOCK_ACTION);

    return 0;
}

int BoltLockManager::ActuatorMovementTimerEventHandler(void * data)
{
    BoltLockManager * lock   = reinterpret_cast<BoltLockManager *>(data);
    Action_t actionCompleted = INVALID_ACTION;

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

            printf("Auto Re-lock enabled. Will be triggered in %lu seconds\n", lock->mAutoLockDuration);
        }
    }

    return 0;
}
