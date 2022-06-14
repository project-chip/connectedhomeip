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

#pragma once

#include <lib/core/ClusterEnums.h>

#include <zephyr/zephyr.h>

#include <cstdint>

class AppEvent;

class BoltLockManager
{
public:
    enum class State : uint8_t
    {
        kLockingInitiated = 0,
        kLockingCompleted,
        kUnlockingInitiated,
        kUnlockingCompleted,
    };

    using OperationSource     = chip::app::Clusters::DoorLock::DlOperationSource;
    using StateChangeCallback = void (*)(State, OperationSource);

    static constexpr uint32_t kActuatorMovementTimeMs = 2000;

    void Init(StateChangeCallback callback);

    State GetState() const { return mState; }
    bool IsLocked() const { return mState == State::kLockingCompleted; }

    void Lock(OperationSource source);
    void Unlock(OperationSource source);

private:
    void SetState(State state, OperationSource source);

    static void ActuatorTimerEventHandler(k_timer * timer);
    static void ActuatorAppEventHandler(AppEvent * aEvent);
    friend BoltLockManager & BoltLockMgr();

    State mState                             = State::kLockingCompleted;
    StateChangeCallback mStateChangeCallback = nullptr;
    OperationSource mActuatorOperationSource = OperationSource::kButton;
    k_timer mActuatorTimer                   = {};

    static BoltLockManager sLock;
};

inline BoltLockManager & BoltLockMgr()
{
    return BoltLockManager::sLock;
}
