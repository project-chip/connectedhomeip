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

#include "PumpManager.h"

#include "AppConfig.h"
#include "AppTask.h"
#include "FreeRTOS.h"

#define ACTUATOR_MOVEMENT_PERIOS_MS 500

PumpManager PumpManager::sPump;

int PumpManager::Init()
{
    int ret = 0;

    mTimerHandle = xTimerCreate("BLT_TIMER", pdMS_TO_TICKS(ACTUATOR_MOVEMENT_PERIOS_MS), pdFALSE, this, TimerEventHandler);
    if (NULL == mTimerHandle)
    {
        PLAT_LOG("failed to create pump timer");
        while (1)
            ;
    }

    mState               = kState_StopCompleted;
    mAutoStartTimerArmed = false;
    mAutoRestart         = false;
    mAutoStartDuration   = 0;

    return ret;
}

void PumpManager::SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

bool PumpManager::IsActionInProgress()
{
    return (mState == kState_StartInitiated || mState == kState_StopInitiated);
}

bool PumpManager::IsStopped()
{
    return (mState == kState_StopCompleted);
}

void PumpManager::EnableAutoRestart(bool aOn)
{
    mAutoRestart = aOn;
}

void PumpManager::SetAutoStartDuration(uint32_t aDurationInSecs)
{
    mAutoStartDuration = aDurationInSecs;
}

bool PumpManager::InitiateAction(int32_t aActor, Action_t aAction)
{
    bool action_initiated = false;
    State_t new_state;

    // Initiate Start/Stop Action only when the previous one is complete.
    if (mState == kState_StartCompleted && aAction == STOP_ACTION)
    {
        action_initiated = true;

        new_state = kState_StopInitiated;
    }
    else if (mState == kState_StopCompleted && aAction == START_ACTION)
    {
        action_initiated = true;
        mCurrentActor    = aActor;
        new_state        = kState_StartInitiated;
    }

    if (action_initiated)
    {
        if (mAutoStartTimerArmed && new_state == kState_StartInitiated)
        {
            // If auto start timer has been armed and someone initiates start,
            // cancel the timer and continue as normal.
            mAutoStartTimerArmed = false;

            CancelTimer();
        }

        PumpTimer(ACTUATOR_MOVEMENT_PERIOS_MS);

        // Since the timer started successfully, update the state and trigger callback
        mState = new_state;

        if (mActionInitiated_CB)
        {
            mActionInitiated_CB(aAction, aActor);
        }
    }

    return action_initiated;
}

void PumpManager::PumpTimer(uint32_t aTimeoutMs)
{
    xTimerChangePeriod(mTimerHandle, pdMS_TO_TICKS(aTimeoutMs), 100);
    xTimerStart(mTimerHandle, 100);
}

void PumpManager::CancelTimer(void)
{
    xTimerStop(mTimerHandle, 100);
}

void PumpManager::TimerEventHandler(TimerHandle_t aTimer)
{
    PumpManager * pump = static_cast<PumpManager *>(pvTimerGetTimerID(aTimer));

    // The timer event handler will be called in the context of the timer task
    // once sPumpTimer expires. Post an event to apptask queue with the actual handler
    // so that the event can be handled in the context of the apptask.
    AppEvent event;
    event.Type                   = AppEvent::kEventType_AppEvent;
    event.PumpStateEvent.Context = static_cast<PumpManager *>(pump);
    if (pump->mAutoStartTimerArmed)
    {
        event.Handler = AutoRestartTimerEventHandler;
    }
    else
    {
        event.Handler = ActuatorMovementTimerEventHandler;
    }
    GetAppTask().PostEvent(&event);
}

void PumpManager::AutoRestartTimerEventHandler(AppEvent * aEvent)
{
    PumpManager * pump = static_cast<PumpManager *>(aEvent->PumpStateEvent.Context);
    int32_t actor      = 0;

    // Make sure auto start timer is still armed.
    if (!pump->mAutoStartTimerArmed)
    {
        return;
    }

    pump->mAutoStartTimerArmed = false;

    PLAT_LOG("Auto Re-Start has been triggered!");

    pump->InitiateAction(actor, START_ACTION);
}

void PumpManager::ActuatorMovementTimerEventHandler(AppEvent * aEvent)
{
    Action_t actionCompleted = INVALID_ACTION;

    PumpManager * pump = static_cast<PumpManager *>(aEvent->PumpStateEvent.Context);

    if (pump->mState == kState_StartInitiated)
    {
        pump->mState    = kState_StartCompleted;
        actionCompleted = START_ACTION;
    }
    else if (pump->mState == kState_StopInitiated)
    {
        pump->mState    = kState_StopCompleted;
        actionCompleted = STOP_ACTION;
    }

    if (actionCompleted != INVALID_ACTION)
    {
        if (pump->mActionCompleted_CB)
        {
            pump->mActionCompleted_CB(actionCompleted, pump->mCurrentActor);
        }

        if (pump->mAutoRestart && actionCompleted == STOP_ACTION)
        {
            // Start the timer for auto restart
            pump->PumpTimer(pump->mAutoStartDuration * 1000);

            pump->mAutoStartTimerArmed = true;

            PLAT_LOG("Auto Re-start enabled. Will be triggered in %u seconds", pump->mAutoStartDuration);
        }
    }
}
