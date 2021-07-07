/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <support/logging/CHIPLogging.h>

// mbed-os headers
#include "drivers/Timeout.h"
#include "platform/Callback.h"

static mbed::Timeout sLockTimer;

BoltLockManager BoltLockManager::sLock;

void BoltLockManager::Init()
{
    mState              = kState_LockingCompleted;
    mAutoLockTimerArmed = false;
    mAutoRelock         = false;
    mAutoLockDuration   = 0;
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
        mCurrentActor    = aActor;
        new_state        = kState_UnlockingInitiated;
    }
    else if (mState == kState_UnlockingCompleted && aAction == LOCK_ACTION)
    {
        action_initiated = true;
        mCurrentActor    = aActor;
        new_state        = kState_LockingInitiated;
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

        StartTimer(MBED_CONF_APP_ACTUATOR_MOVEMENT_PERIOD_MS);

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
    auto chronoTimeoutMs = std::chrono::duration<uint32_t, std::milli>(aTimeoutMs);
    sLockTimer.attach(mbed::callback(this, &BoltLockManager::TimerEventHandler), chronoTimeoutMs);
}

void BoltLockManager::CancelTimer(void)
{
    sLockTimer.detach();
}

void BoltLockManager::TimerEventHandler(void)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = this;
    event.Handler            = mAutoLockTimerArmed ? AutoReLockTimerEventHandler : ActuatorMovementTimerEventHandler;
    GetAppTask().PostEvent(&event);
}

void BoltLockManager::AutoReLockTimerEventHandler(AppEvent * aEvent)
{
    BoltLockManager * lock = static_cast<BoltLockManager *>(aEvent->TimerEvent.Context);
    int32_t actor          = 0;

    // Make sure auto lock timer is still armed.
    if (!lock->mAutoLockTimerArmed)
        return;

    lock->mAutoLockTimerArmed = false;

    ChipLogProgress(NotSpecified, "Auto Re-Lock has been triggered!");

    lock->InitiateAction(actor, LOCK_ACTION);
}

void BoltLockManager::ActuatorMovementTimerEventHandler(AppEvent * aEvent)
{
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
            lock->mActionCompleted_CB(actionCompleted, lock->mCurrentActor);
        }

        if (lock->mAutoRelock && actionCompleted == UNLOCK_ACTION)
        {
            // Start the timer for auto relock
            lock->StartTimer(lock->mAutoLockDuration * 1000);

            lock->mAutoLockTimerArmed = true;

            ChipLogProgress(NotSpecified, "Auto Re-lock enabled. Will be triggered in %u seconds", lock->mAutoLockDuration);
        }
    }
}
