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

#include "AppTask.h"
#include "FreeRTOS.h"

#include "app_config.h"

LightingManager LightingManager::sLight;

TimerHandle_t sLightTimer; // FreeRTOS app sw timer.

int LightingManager::Init()
{
    // Create FreeRTOS sw timer for light timer.

    sLightTimer = xTimerCreate("LightTmr",       // Just a text name, not used by the RTOS kernel
                               1,                // == default timer period (mS)
                               false,            // no timer reload (==one-shot)
                               (void *) this,    // init timer id = light obj context
                               TimerEventHandler // timer callback handler
    );

    if (sLightTimer == NULL)
    {
        K32W_LOG("light timer create failed");
        assert(0);
    }

    mState                = kState_TurnOffCompleted;
    mAutoTurnOnTimerArmed = false;
    mAutoTurnOn           = false;
    mAutoTurnOnDuration   = 0;

    return 0;
}

void LightingManager::SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

bool LightingManager::IsActionInProgress()
{
    return (mState == kState_TurnOnInitiated || mState == kState_TurnOffInitiated) ? true : false;
}

bool LightingManager::IsTurnedOff()
{
    return (mState == kState_TurnOffCompleted) ? true : false;
}

void LightingManager::EnableAutoTurnOn(bool aOn)
{
    mAutoTurnOn = aOn;
}

void LightingManager::SetAutoTurnOnDuration(uint32_t aDurationInSecs)
{
    mAutoTurnOnDuration = aDurationInSecs;
}

bool LightingManager::InitiateAction(int32_t aActor, Action_t aAction)
{
    bool action_initiated = false;
    State_t new_state;

    // Initiate ON/OFF Action only when the previous one is complete.
    if (mState == kState_TurnOnCompleted && aAction == TURNOFF_ACTION)
    {
        action_initiated = true;

        new_state = kState_TurnOffInitiated;
    }
    else if (mState == kState_TurnOffCompleted && aAction == TURNON_ACTION)
    {
        action_initiated = true;

        new_state = kState_TurnOnInitiated;
    }

    if (action_initiated)
    {
        if (mAutoTurnOnTimerArmed && new_state == kState_TurnOnInitiated)
        {
            // If auto turnon timer has been armed and someone initiates turn on the ligth,
            // cancel the timer and continue as normal.
            mAutoTurnOnTimerArmed = false;

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
        K32W_LOG("light timer already started!");
        // appError(err);
        CancelTimer();
    }

    // timer is not active, change its period to required value.
    // This also causes the timer to start.  FreeRTOS- Block for a maximum of
    // 100 ticks if the  change period command cannot immediately be sent to the
    // timer command queue.
    if (xTimerChangePeriod(sLightTimer, aTimeoutMs / portTICK_PERIOD_MS, 100) != pdPASS)
    {
        K32W_LOG("light timer start() failed");
        // appError(err);
    }
}

void LightingManager::CancelTimer(void)
{
    if (xTimerStop(sLightTimer, 0) == pdFAIL)
    {
        K32W_LOG("light timer stop() failed");
        // appError(err);
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

    if (light->mAutoTurnOnTimerArmed)
    {
        event.Handler = AutoReTurnOnTimerEventHandler;
        GetAppTask().PostEvent(&event);
    }
    else
    {
        event.Handler = ActuatorMovementTimerEventHandler;
        GetAppTask().PostEvent(&event);
    }
}

void LightingManager::AutoReTurnOnTimerEventHandler(AppEvent * aEvent)
{
    LightingManager * light = static_cast<LightingManager *>(aEvent->TimerEvent.Context);
    int32_t actor           = 0;

    // Make sure auto light timer is still armed.
    if (!light->mAutoTurnOnTimerArmed)
    {
        return;
    }

    light->mAutoTurnOnTimerArmed = false;

    K32W_LOG("Auto Turn On has been triggered!");

    light->InitiateAction(actor, TURNON_ACTION);
}

void LightingManager::ActuatorMovementTimerEventHandler(AppEvent * aEvent)
{
    Action_t actionCompleted = INVALID_ACTION;

    LightingManager * light = static_cast<LightingManager *>(aEvent->TimerEvent.Context);

    if (light->mState == kState_TurnOnInitiated)
    {
        light->mState   = kState_TurnOnCompleted;
        actionCompleted = TURNON_ACTION;
    }
    else if (light->mState == kState_TurnOffInitiated)
    {
        light->mState   = kState_TurnOffCompleted;
        actionCompleted = TURNOFF_ACTION;
    }

    if (actionCompleted != INVALID_ACTION)
    {
        if (light->mActionCompleted_CB)
        {
            light->mActionCompleted_CB(actionCompleted);
        }

        if (light->mAutoTurnOn && actionCompleted == TURNOFF_ACTION)
        {
            // Start the timer for auto turn on
            light->StartTimer(light->mAutoTurnOnDuration * 1000);

            light->mAutoTurnOnTimerArmed = true;

            K32W_LOG("Auto Turn On enabled. Will be triggered in %u seconds", light->mAutoTurnOnDuration);
        }
    }
}
