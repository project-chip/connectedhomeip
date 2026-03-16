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

#include <app/clusters/switch-server/switch-server.h>

#include <stdbool.h>
#include <stdint.h>

#include "AppEvent.h"

#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support
#include <app/util/basic-types.h>

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>

using namespace ::chip;

class LightSwitchManager
{
public:
    enum Action_t
    {
        SWITCH_ON_ACTION = 0,
        SWITCH_OFF_ACTION,
        INVALID_ACTION
    } Action;

    enum State_t
    {
        kState_SwitchOffInitiated = 0,
        kState_SwitchOffCompleted,
        kState_SwitchOnInitiated,
        kState_SwitchOnCompleted,
    } State;

    CHIP_ERROR Init();

    bool InitiateAction(int32_t aActor, Action_t aAction);

    typedef void (*Callback_fn_initiated)(Action_t, int32_t aActor);
    typedef void (*Callback_fn_completed)(Action_t);
    void SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB);

    bool SwitchOn(chip::EndpointId endpointId);
    bool SwitchOff(chip::EndpointId endpointId);

    void IdentifyEventHandler();

private:
    friend LightSwitchManager & LightSwitchMgr(void);
    State_t mState;

    Callback_fn_initiated mActionInitiated_CB;
    Callback_fn_completed mActionCompleted_CB;

    void CancelTimer(void);
    void StartTimer(uint32_t aTimeoutMs);

    static void TimerEventHandler(TimerHandle_t xTimer);
    static void ActuatorMovementTimerEventHandler(AppEvent * aEvent);

    static LightSwitchManager sLightSwitch;
};

inline LightSwitchManager & LightSwitchMgr(void)
{
    return LightSwitchManager::sLightSwitch;
}
