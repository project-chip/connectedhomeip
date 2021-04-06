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

#pragma once

#include <cstdint>

#include "AppEvent.h"

class BoltLockManager
{
public:
    enum Action_t
    {
        LOCK_ACTION = 0,
        UNLOCK_ACTION,

        INVALID_ACTION
    };

    enum State_t
    {
        kState_LockingInitiated = 0,
        kState_LockingCompleted,
        kState_UnlockingInitiated,
        kState_UnlockingCompleted,
    };

    void Init();
    bool IsUnlocked();
    void EnableAutoRelock(bool aOn);
    void SetAutoLockDuration(uint32_t aDurationInSecs);
    bool IsActionInProgress();
    bool InitiateAction(int32_t aActor, Action_t aAction);

    typedef void (*Callback_fn_initiated)(Action_t, int32_t aActor);
    typedef void (*Callback_fn_completed)(Action_t, int32_t aActor);
    void SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB);

private:
    friend BoltLockManager & BoltLockMgr(void);
    State_t mState;

    Callback_fn_initiated mActionInitiated_CB;
    Callback_fn_completed mActionCompleted_CB;

    bool mAutoRelock;
    uint32_t mAutoLockDuration;
    bool mAutoLockTimerArmed;
    int32_t mCurrentActor;

    void CancelTimer(void);
    void StartTimer(uint32_t aTimeoutMs);

    void TimerEventHandler(void);
    static void AutoReLockTimerEventHandler(AppEvent * aEvent);
    static void ActuatorMovementTimerEventHandler(AppEvent * aEvent);

    static BoltLockManager sLock;
};

inline BoltLockManager & BoltLockMgr(void)
{
    return BoltLockManager::sLock;
}
