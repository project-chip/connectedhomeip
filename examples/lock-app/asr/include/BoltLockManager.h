/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#pragma once

#include <stdint.h>

#include "AppEvent.h"

#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support

#include <lib/core/CHIPError.h>

class BoltLockManager
{
public:
    enum class Action
    {
        kLock = 0,
        kUnlock,
        KInvalid
    };

    enum class State
    {
        kLockingInitiated = 0,
        kLockingCompleted,
        kUnlockingInitiated,
        kUnlockingCompleted,
    };

    CHIP_ERROR Init();
    bool IsUnlocked();
    void EnableAutoRelock(bool aOn);
    void SetAutoLockDuration(uint32_t aDurationInSecs);
    bool IsActionInProgress();
    bool InitiateAction(int32_t aActor, Action aAction);

    typedef void (*Callback_fn_initiated)(Action, int32_t aActor);
    typedef void (*Callback_fn_completed)(Action);
    void SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB);

private:
    friend BoltLockManager & BoltLockMgr(void);
    State mState = State::kUnlockingCompleted;

    Callback_fn_initiated mActionInitiated_CB;
    Callback_fn_completed mActionCompleted_CB;

    bool mAutoRelock           = false;
    uint32_t mAutoLockDuration = 0;
    bool mAutoLockTimerArmed   = false;

    void CancelTimer(void);
    void StartTimer(uint32_t aTimeoutMs);

    static void TimerEventHandler(TimerHandle_t xTimer);
    static void AutoReLockTimerEventHandler(AppEvent * aEvent);
    static void ActuatorMovementTimerEventHandler(AppEvent * aEvent);

    static BoltLockManager sLock;
};

inline BoltLockManager & BoltLockMgr(void)
{
    return BoltLockManager::sLock;
}
