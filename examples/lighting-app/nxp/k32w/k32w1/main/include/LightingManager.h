/*
 *
 *    Copyright (c) 2021 Google LLC.
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

#include "AppEvent.h"

#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support

class LightingManager
{
public:
    enum Action_t
    {
        TURNON_ACTION = 0,
        TURNOFF_ACTION,
        DIM_ACTION,
        INVALID_ACTION
    } Action;

    enum State_t
    {
        kState_On = 0,
        kState_Off,
    } State;

    static const uint8_t kLevel_Max = 254;
    static const uint8_t kLevel_Min = 0;

    int Init();
    bool IsTurnedOff();
    uint8_t GetDimLevel();
    bool InitiateAction(int32_t aActor, Action_t aAction, uint8_t kValue);

    typedef void (*Callback_fn_initiated)(Action_t, int32_t aActor);
    typedef void (*Callback_fn_completed)(Action_t, uint8_t level);
    void SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB);
    void SetState(bool state);
    void SetDimLevel(uint8_t level);

private:
    friend LightingManager & LightingMgr(void);
    State_t mState;
    uint8_t mLevel;

    Callback_fn_initiated mActionInitiated_CB;
    Callback_fn_completed mActionCompleted_CB;

    static LightingManager sLight;
};

inline LightingManager & LightingMgr(void)
{
    return LightingManager::sLight;
}
