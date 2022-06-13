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

#include <zephyr/logging/log.h>
#include <zephyr/zephyr.h>

LOG_MODULE_DECLARE(app, CONFIG_MATTER_LOG_LEVEL);

static k_timer sStartTimer;

PumpManager PumpManager::sPump;

void PumpManager::Init()
{
    k_timer_init(&sStartTimer, &PumpManager::TimerEventHandler, nullptr);
    k_timer_user_data_set(&sStartTimer, this);

    mState               = kState_StartCompleted;
    mAutoStartTimerArmed = false;
    mAutoRestart         = false;
    mAutoStartDuration   = 0;
}

void PumpManager::SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

bool PumpManager::IsActionInProgress()
{
    return (mState == kState_StartInitiated || mState == kState_StartInitiated) ? true : false;
}

bool PumpManager::IsStopped()
{
    return (mState == kState_StopCompleted) ? true : false;
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
        if (mAutoStartTimerArmed && new_state == kState_StartInitiated)
        {
            // If auto start timer has been armed and someone initiates stop,
            // cancel the timer and continue as normal.
            mAutoStartTimerArmed = false;

            CancelTimer();
        }

        StartTimer(PUMP_START_PERIOS_MS);

        // Since the timer started successfully, update the state and trigger callback
        mState = new_state;

        if (mActionInitiated_CB)
        {
            mActionInitiated_CB(aAction, aActor);
        }
    }

    return action_initiated;
}

void PumpManager::StartTimer(uint32_t aTimeoutMs)
{
    k_timer_start(&sStartTimer, K_MSEC(aTimeoutMs), K_NO_WAIT);
}

void PumpManager::CancelTimer(void)
{
    k_timer_stop(&sStartTimer);
}

void PumpManager::TimerEventHandler(k_timer * timer)
{
    PumpManager * pump = static_cast<PumpManager *>(k_timer_user_data_get(timer));

    // The timer event handler will be called in the context of the timer task
    // once sStartTimer expires. Post an event to apptask queue with the actual handler
    // so that the event can be handled in the context of the apptask.
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = pump;
    event.Handler            = pump->mAutoStartTimerArmed ? AutoRestartTimerEventHandler : PumpStartTimerEventHandler;
    GetAppTask().PostEvent(&event);
}

void PumpManager::AutoRestartTimerEventHandler(AppEvent * aEvent)
{
    PumpManager * pump = static_cast<PumpManager *>(aEvent->TimerEvent.Context);
    int32_t actor      = 0;

    // Make sure auto start timer is still armed.
    if (!pump->mAutoStartTimerArmed)
        return;

    pump->mAutoStartTimerArmed = false;

    LOG_INF("Auto Re-Start has been triggered!");

    pump->InitiateAction(actor, START_ACTION);
}

void PumpManager::PumpStartTimerEventHandler(AppEvent * aEvent)
{
    Action_t actionCompleted = INVALID_ACTION;

    PumpManager * pump = static_cast<PumpManager *>(aEvent->TimerEvent.Context);

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

        if (pump->mAutoRestart && actionCompleted == START_ACTION)
        {
            // Start the timer for auto restart
            pump->StartTimer(pump->mAutoStartDuration * 1000);

            pump->mAutoStartTimerArmed = true;

            LOG_INF("Auto Re-start enabled. Will be triggered in %u seconds", pump->mAutoStartDuration);
        }
    }
}
