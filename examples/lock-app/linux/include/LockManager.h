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

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <functional>

class LockManager
{
public:
    enum Action_t
    {
        LOCK = 0,
        UNLOCK,

        INVALID_ACTION
    } Action;

    enum State_t
    {
        kState_Locked = 0,
        kState_Unlocked,
    } State;

    void Init();
    //bool InitiateAction(Action_t aAction);

    bool Lock(const char* pin);
    bool Unlock(const char* pin);

    //using LockCallback_fn = std::function<void(Action_t)>;

    //void SetCallbacks(LockCallback_fn aActionInitiated_CB, LockCallback_fn aActionCompleted_CB);

private:
    friend LockManager & LockMgr(void);

    //State_t mState;

    bool mLocked;

    //LockCallback_fn mActionInitiated_CB;
    //LockCallback_fn mActionCompleted_CB;

    static LockManager sLock;
};

inline LockManager & LockMgr(void)
{
    return LockManager::sLock;
}
