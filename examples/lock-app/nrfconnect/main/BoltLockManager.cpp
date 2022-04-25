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
#include "AppEvent.h"
#include "AppTask.h"

BoltLockManager BoltLockManager::sLock;

void BoltLockManager::Init(StateChangeCallback callback)
{
    mStateChangeCallback = callback;

    k_timer_init(&mActuatorTimer, &BoltLockManager::ActuatorTimerEventHandler, nullptr);
    k_timer_user_data_set(&mActuatorTimer, this);
}

void BoltLockManager::Lock(OperationSource source)
{
    VerifyOrReturn(mState != State::kLockingCompleted);
    SetState(State::kLockingInitiated, source);

    mActuatorOperationSource = source;
    k_timer_start(&mActuatorTimer, K_MSEC(kActuatorMovementTimeMs), K_NO_WAIT);
}

void BoltLockManager::Unlock(OperationSource source)
{
    VerifyOrReturn(mState != State::kUnlockingCompleted);
    SetState(State::kUnlockingInitiated, source);

    mActuatorOperationSource = source;
    k_timer_start(&mActuatorTimer, K_MSEC(kActuatorMovementTimeMs), K_NO_WAIT);
}

void BoltLockManager::ActuatorTimerEventHandler(k_timer * timer)
{
    // The timer event handler is called in the context of the system clock ISR.
    // Post an event to the application task queue to process the event in the
    // context of the application thread.

    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = static_cast<BoltLockManager *>(k_timer_user_data_get(timer));
    event.Handler            = BoltLockManager::ActuatorAppEventHandler;
    GetAppTask().PostEvent(&event);
}

void BoltLockManager::ActuatorAppEventHandler(AppEvent * aEvent)
{
    BoltLockManager * lock = static_cast<BoltLockManager *>(aEvent->TimerEvent.Context);

    switch (lock->mState)
    {
    case State::kLockingInitiated:
        lock->SetState(State::kLockingCompleted, lock->mActuatorOperationSource);
        break;
    case State::kUnlockingInitiated:
        lock->SetState(State::kUnlockingCompleted, lock->mActuatorOperationSource);
        break;
    default:
        break;
    }
}

void BoltLockManager::SetState(State state, OperationSource source)
{
    mState = state;

    if (mStateChangeCallback != nullptr)
    {
        mStateChangeCallback(state, source);
    }
}
