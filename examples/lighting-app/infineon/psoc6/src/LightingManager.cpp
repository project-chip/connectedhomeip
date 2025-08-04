/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "LightingManager.h"

#include "AppConfig.h"
#include "AppTask.h"
#include <FreeRTOS.h>

LightingManager LightingManager::sLight;

TimerHandle_t sLightTimer;

CHIP_ERROR LightingManager::Init()
{
    // Create FreeRTOS sw timer for light timer.
    sLightTimer = xTimerCreate("lightTmr",       // Just a text name, not used by the RTOS kernel
                               1,                // == default timer period (mS)
                               false,            // no timer reload (==one-shot)
                               (void *) this,    // init timer id = light obj context
                               TimerEventHandler // timer callback handler
    );

    if (sLightTimer == NULL)
    {
        PSOC6_LOG("sLockTimer timer create failed");
        return APP_ERROR_CREATE_TIMER_FAILED;
    }

    mState                 = kState_OffCompleted;
    mAutoTurnOffTimerArmed = false;
    mAutoTurnOff           = false;
    mAutoTurnOffDuration   = 0;

    return CHIP_NO_ERROR;
}

void LightingManager::SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

bool LightingManager::IsActionInProgress()
{
    return (mState == kState_OffInitiated || mState == kState_OnInitiated);
}

bool LightingManager::IsLightOn()
{
    return (mState == kState_OnCompleted);
}

void LightingManager::EnableAutoTurnOff(bool aOn)
{
    mAutoTurnOff = aOn;
}

void LightingManager::SetAutoTurnOffDuration(uint32_t aDurationInSecs)
{
    mAutoTurnOffDuration = aDurationInSecs;
}

bool LightingManager::InitiateAction(int32_t aActor, Action_t aAction)
{
    bool action_initiated = false;
    State_t new_state;

    // Initiate Turn On/Off Action only when the previous one is complete.
    if (mState == kState_OffCompleted && aAction == ON_ACTION)
    {
        action_initiated = true;

        new_state = kState_OnInitiated;
    }
    else if (mState == kState_OnCompleted && aAction == OFF_ACTION)
    {
        action_initiated = true;

        new_state = kState_OffInitiated;
    }

    if (action_initiated)
    {
        if (mAutoTurnOffTimerArmed && new_state == kState_OffInitiated)
        {
            // If auto turn off timer has been armed and someone initiates turning off,
            // cancel the timer and continue as normal.
            mAutoTurnOffTimerArmed = false;

            CancelTimer();
        }

        StartTimer(ACTUATOR_MOVEMENT_PERIOS_MS);

        // Since the timer started successfully, update the state and trigger callback
        mState = new_state;

        if (mActionInitiated_CB)
        {
            mActionInitiated_CB(aAction, aActor);
        }
    }

    return action_initiated;
}

void LightingManager::StartTimer(uint32_t aTimeoutMs)
{
    if (xTimerIsTimerActive(sLightTimer))
    {
        PSOC6_LOG("app timer already started!");
        CancelTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(sLightTimer, (aTimeoutMs / portTICK_PERIOD_MS), 100) != pdPASS)
    {
        PSOC6_LOG("sLockTimer timer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }
}

void LightingManager::CancelTimer(void)
{
    if (xTimerStop(sLightTimer, 0) == pdFAIL)
    {
        PSOC6_LOG("sLightTimer stop() failed");
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }
}

void LightingManager::TimerEventHandler(TimerHandle_t xTimer)
{
    // Get light obj context from timer id.
    LightingManager * light = static_cast<LightingManager *>(pvTimerGetTimerID(xTimer));

    // The timer event handler will be called in the context of the timer task
    // once sLightTimer expires. Post an event to apptask queue with the actual handler
    // so that the event can be handled in the context of the apptask.
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = light;
    if (light->mAutoTurnOffTimerArmed)
    {
        event.Handler = AutoTurnOffTimerEventHandler;
    }
    else
    {
        event.Handler = ActuatorMovementTimerEventHandler;
    }
    GetAppTask().PostEvent(&event);
}

void LightingManager::AutoTurnOffTimerEventHandler(AppEvent * aEvent)
{
    LightingManager * light = static_cast<LightingManager *>(aEvent->TimerEvent.Context);
    int32_t actor           = 0;

    // Make sure auto turn off timer is still armed.
    if (!light->mAutoTurnOffTimerArmed)
    {
        return;
    }

    light->mAutoTurnOffTimerArmed = false;

    PSOC6_LOG("Auto Turn Off has been triggered!");

    light->InitiateAction(actor, OFF_ACTION);
}

void LightingManager::ActuatorMovementTimerEventHandler(AppEvent * aEvent)
{
    Action_t actionCompleted = INVALID_ACTION;

    LightingManager * light = static_cast<LightingManager *>(aEvent->TimerEvent.Context);

    if (light->mState == kState_OffInitiated)
    {
        light->mState   = kState_OffCompleted;
        actionCompleted = OFF_ACTION;
    }
    else if (light->mState == kState_OnInitiated)
    {
        light->mState   = kState_OnCompleted;
        actionCompleted = ON_ACTION;
    }

    if (actionCompleted != INVALID_ACTION)
    {
        if (light->mActionCompleted_CB)
        {
            light->mActionCompleted_CB(actionCompleted);
        }

        if (light->mAutoTurnOff && actionCompleted == ON_ACTION)
        {
            // Start the timer for auto turn off
            light->StartTimer(light->mAutoTurnOffDuration * 1000);

            light->mAutoTurnOffTimerArmed = true;

            PSOC6_LOG("Auto Turn off enabled. Will be triggered in %u seconds", light->mAutoTurnOffDuration);
        }
    }
}
