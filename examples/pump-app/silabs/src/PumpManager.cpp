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

#define ACTUATOR_MOVEMENT_PERIOS_MS 500

PumpManager PumpManager::sPump;

CHIP_ERROR PumpManager::Init()
{
    CHIP_ERROR ret = CHIP_NO_ERROR;

    mTimerHandle = osTimerNew(TimerEventHandler, osTimerOnce, this, NULL);
    if (NULL == mTimerHandle)
    {
        return CHIP_ERROR_INTERNAL;
    }

    mState = kState_StopCompleted;

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

bool PumpManager::InitiateAction(int32_t aActor, Action_t aAction)
{
    bool action_initiated = false;
    State_t new_state;

    // Initiate Start/Stop Action only when the previous one is complete.
    if (mState == kState_StartCompleted && aAction == STOP_ACTION)
    {
        action_initiated = true;
        mCurrentActor    = aActor;
        new_state        = kState_StopInitiated;
    }
    else if (mState == kState_StopCompleted && aAction == START_ACTION)
    {
        action_initiated = true;
        mCurrentActor    = aActor;
        new_state        = kState_StartInitiated;
    }

    if (action_initiated)
    {

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
    osTimerStart(mTimerHandle, pdMS_TO_TICKS(aTimeoutMs));
}

void PumpManager::CancelTimer(void)
{
    osTimerStop(mTimerHandle);
}

void PumpManager::TimerEventHandler(void * timerCbArg)
{
    PumpManager * pump = static_cast<PumpManager *>(timerCbArg);

    // The timer event handler will be called in the context of the timer task
    // once sPumpTimer expires. Post an event to apptask queue with the actual handler
    // so that the event can be handled in the context of the apptask.
    AppEvent event;
    event.Type              = AppEvent::kEventType_PumpTimer;
    event.PumpEvent.Context = static_cast<void *>(pump);
    event.Handler           = ActuatorMovementTimerEventHandler;

    AppTask::GetAppTask().PostEvent(&event);
}

void PumpManager::ActuatorMovementTimerEventHandler(AppEvent * aEvent)
{
    Action_t actionCompleted = INVALID_ACTION;

    PumpManager * pump = static_cast<PumpManager *>(aEvent->PumpEvent.Context);

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
    }
}
