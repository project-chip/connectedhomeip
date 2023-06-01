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

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "AppEvent.h"

#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support
#include <app/clusters/on-off-server/on-off-server.h>

#include <lib/core/CHIPError.h>

class OnOffPlugManager
{
public:
    enum Action_t
    {
        OFF_ACTION = 0,
        ON_ACTION  = 1,
        INVALID_ACTION
    } Action;

    CHIP_ERROR Init();
    bool IsPlugOn();
    void EnableAutoTurnOff(bool aOn);
    void SetAutoTurnOffDuration(uint32_t aDurationInSecs);
    bool InitiateAction(int32_t aActor, Action_t aAction);

    typedef void (*Callback_fn_initiated)(Action_t, int32_t aActor);
    typedef void (*Callback_fn_completed)(Action_t);
    void SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB);

    static void OnTriggerOffWithEffect(OnOffEffect * effect);

private:
    friend OnOffPlugManager & PlugMgr(void);

    Callback_fn_initiated mActionInitiated_CB;
    Callback_fn_completed mActionCompleted_CB;

    bool mIsOn;
    bool mAutoTurnOff;
    bool mAutoTurnOffTimerArmed;
    bool mOffEffectArmed;
    uint32_t mAutoTurnOffDuration;

    void CancelTimer(void);
    void StartTimer(uint32_t aTimeoutMs);

    static void TimerEventHandler(TimerHandle_t xTimer);
    static void AutoTurnOffTimerEventHandler(AppEvent * aEvent);
    static void OffEffectTimerEventHandler(AppEvent * aEvent);

    static OnOffPlugManager sPlug;
};

inline OnOffPlugManager & PlugMgr(void)
{
    return OnOffPlugManager::sPlug;
}
