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

#include "AppEvent.h"

#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support

class LightingManager
{
public:
    enum Action_t
    {
        ON_ACTION = 0,
        OFF_ACTION,
        LEVEL_ACTION,
        INVALID_ACTION
    } Action;

    enum State_t
    {
        kState_On = 0,
        kState_Off,
    } State;

    int Init();
    bool IsTurnedOn();
    uint8_t GetLevel();
    bool InitiateAction(Action_t aAction, int32_t aActor, uint8_t size, uint8_t * value);

    using LightingCallback_fn = std::function<void(Action_t)>;

    void SetCallbacks(LightingCallback_fn aActionInitiated_CB, LightingCallback_fn aActionCompleted_CB);

private:
    friend LightingManager & LightingMgr(void);
    State_t mState;
    uint8_t mLevel;

    LightingCallback_fn mActionInitiated_CB;
    LightingCallback_fn mActionCompleted_CB;

    void Set(bool aOn);
    void SetLevel(uint8_t aLevel);
    void UpdateLight();

    static LightingManager sLight;
    static void TimerEventHandler(TimerHandle_t xTimer);
};

inline LightingManager & LightingMgr(void)
{
    return LightingManager::sLight;
}
