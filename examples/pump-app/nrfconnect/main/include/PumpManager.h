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

#include <cstdint>

#include "AppEvent.h"

struct k_timer;

class PumpManager
{
public:
    enum Action_t
    {
        START_ACTION = 0,
        STOP_ACTION,

        INVALID_ACTION
    };

    enum State_t
    {
        kState_StartInitiated = 0,
        kState_StartCompleted,
        kState_StopInitiated,
        kState_StopCompleted,
    };

    void Init();
    bool IsStopped();
    void EnableAutoRestart(bool aOn);
    void SetAutoStartDuration(uint32_t aDurationInSecs);
    bool IsActionInProgress();
    bool InitiateAction(int32_t aActor, Action_t aAction);

    int16_t GetMaxPressure();
    uint16_t GetMaxSpeed();
    uint16_t GetMaxFlow();
    int16_t GetMinConstPressure();
    int16_t GetMaxConstPressure();
    int16_t GetMinCompPressure();
    int16_t GetMaxCompPressure();
    uint16_t GetMinConstSpeed();
    uint16_t GetMaxConstSpeed();
    uint16_t GetMinConstFlow();
    uint16_t GetMaxConstFlow();
    int16_t GetMinConstTemp();
    int16_t GetMaxConstTemp();

    typedef void (*Callback_fn_initiated)(Action_t, int32_t aActor);
    typedef void (*Callback_fn_completed)(Action_t, int32_t aActor);
    void SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB);

private:
    friend PumpManager & PumpMgr(void);
    State_t mState;

    Callback_fn_initiated mActionInitiated_CB;
    Callback_fn_completed mActionCompleted_CB;

    bool mAutoRestart;
    uint32_t mAutoStartDuration;
    bool mAutoStartTimerArmed;
    int32_t mCurrentActor;

    void CancelTimer(void);
    void StartTimer(uint32_t aTimeoutMs);

    static void TimerEventHandler(k_timer * timer);
    static void AutoRestartTimerEventHandler(const AppEvent & aEvent);

    static void PumpStartTimerEventHandler(const AppEvent & aEvent);

    static PumpManager sPump;
};

inline PumpManager & PumpMgr(void)
{
    return PumpManager::sPump;
}
