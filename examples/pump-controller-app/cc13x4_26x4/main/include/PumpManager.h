/*
 *
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

#ifndef PUMP_MANAGER_H
#define PUMP_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

#include "AppEvent.h"

#include <FreeRTOS.h>
#include <timers.h>

class PumpManager
{
public:
    enum Action_t
    {
        START_ACTION = 0,
        STOP_ACTION,

        INVALID_ACTION
    } Action;

    enum State_t
    {
        kState_StartInitiated = 0,
        kState_StartCompleted,
        kState_StopInitiated,
        kState_StopCompleted,
    } State;

    int Init();
    bool IsStopped();
    void EnableAutoRestart(bool aOn);
    void SetAutoStartDuration(uint32_t aDurationInSecs);
    bool IsActionInProgress();
    bool InitiateAction(int32_t aActor, Action_t aAction);

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
    TimerHandle_t mTimerHandle;
    int32_t mCurrentActor;

    void CancelTimer(void);
    void PumpTimer(uint32_t aTimeoutMs);

    static void TimerEventHandler(TimerHandle_t aTimer);
    static void AutoRestartTimerEventHandler(AppEvent * aEvent);
    static void ActuatorMovementTimerEventHandler(AppEvent * aEvent);

    static PumpManager sPump;
};

inline PumpManager & PumpMgr(void)
{
    return PumpManager::sPump;
}

#endif // PUMP_MANAGER_H
