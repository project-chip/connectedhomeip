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

#ifndef LIGHTING_MANAGER_H
#define LIGHTING_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

#include <functional>

#include "AppEvent.h"

class LightingManager
{
public:
    enum Action_t
    {
        ON_ACTION = 0,
        OFF_ACTION,

        INVALID_ACTION
    } Action;

    enum State_t
    {
        kState_On = 0,
        kState_Off,
    } State;

    int Init(uint32_t gpioNum);
    bool IsTurnedOn();
    bool InitiateAction(Action_t aAction);

    using LightingCallback_fn = std::function<void(Action_t)>;

    void SetCallbacks(LightingCallback_fn aActionInitiated_CB, LightingCallback_fn aActionCompleted_CB);

private:
    friend LightingManager & LightingMgr(void);
    State_t mState;
    uint32_t mGPIONum;

    LightingCallback_fn mActionInitiated_CB;
    LightingCallback_fn mActionCompleted_CB;

    void Set(bool aOn);

    static LightingManager sLight;
};

inline LightingManager & LightingMgr(void)
{
    return LightingManager::sLight;
}

#endif // LIGHTING_MANAGER_H
