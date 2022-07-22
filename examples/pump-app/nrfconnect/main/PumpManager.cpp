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

int16_t PumpManager::GetMaxPressure()
{
    // 1.6.1. MaxPressure Attribute
    // Range -3276.7 kPa to 3276.7 kPa (steps of 0.1 kPa)
    // -3276.8 is invalid value - perhaps 'null'

    // Return 2000.0 kPa as Max Pressure
    return 20000;
}

uint16_t PumpManager::GetMaxSpeed()
{
    // 1.6.2. MaxSpeed Attribute
    // Range 0 RPM to 65534 RPM (steps of 1 RPM)
    // 65535 is invalid value - perhaps 'null'

    // Return 1000 RPM as MaxSpeed
    return 1000;
}

uint16_t PumpManager::GetMaxFlow()
{
    // 1.6.3. MaxFlow Attribute
    // Range 0 m3/h to 6553.4 m3/h (steps of 0.1 m3/h)
    // 6553.5 m3/h is invalid value - perhaps 'null'

    // Return 200.0 m3/h as MaxFlow
    return 2000;
}

int16_t PumpManager::GetMinConstPressure()
{
    // 1.6.4. MinConstPressure Attribute
    // Range -3276.7 kPa to 3276.7 kPa (steps of 0.1 kPa)
    // -3276.8 is invalid value - perhaps 'null'

    // Return -100.0 kPa as MinConstPressure
    return -1000;
}

int16_t PumpManager::GetMaxConstPressure()
{
    // 1.6.5. MaxConstPressure Attribute
    // Range -3276.7 kPa to 3276.7 kPa (steps of 0.1 kPa)
    // -3276.8 is invalid value - perhaps 'null'

    // Return 100.0 kPa as MaxConstPressure
    return 1000;
}

int16_t PumpManager::GetMinCompPressure()
{
    // 1.6.6. MinCompPressure Attribute
    // Range -3276.7 kPa to 3276.7 kPa (steps of 0.1 kPa)
    // -3276.8 is invalid value - perhaps 'null'

    // Return -20.0 kPa as MinCompPressure
    return -200;
}

int16_t PumpManager::GetMaxCompPressure()
{
    // 1.6.7. MaxCompPressure Attribute
    // Range -3276.7 kPa to 3276.7 kPa (steps of 0.1 kPa)
    // -3276.8 is invalid value - perhaps 'null'

    // Return 20.0 kPa as MaxCompPressure
    return 200;
}

uint16_t PumpManager::GetMinConstSpeed()
{
    // 1.6.8. MinConstSpeed Attribute
    // Range 0 to 65534 RPM (steps of 1 RPM)
    // 65535 RPM is invalid valud - perhaps 'null'

    // Return 200 RPM as MinConstSpeed
    return 200;
}

uint16_t PumpManager::GetMaxConstSpeed()
{
    // 1.6.9. MaxConstSpeed Attribute
    // Range 0 to 65534 RPM (steps of 1 RPM)
    // 65535 RPM is invalid valud - perhaps 'null'

    // Return 2000 RPM as MaxConstSpeed
    return 2000;
}

uint16_t PumpManager::GetMinConstFlow()
{
    // 1.6.10. MinConstFlow Attribute
    // Range 0 m3/h to 6553.4 m3/h (steps of 0.1 m3/h)
    // 6553.5 m3/h is invalid value - perhaps 'null'

    // Return 12.5 m3/h as MinConstFlow
    return 125;
}

uint16_t PumpManager::GetMaxConstFlow()
{
    // 1.6.11. MaxConstFlow Attribute
    // Range 0 m3/h to 6553.4 m3/h (steps of 0.1 m3/h)
    // 6553.5 m3/h is invalid value - perhaps 'null'

    // Return 655.7 m3/h as MaxConstFlow
    return 6557;
}

int16_t PumpManager::GetMinConstTemp()
{
    // 1.6.12. MinConstTemp Attribute
    // Range -273.15 C to 327.67 C (steps of 0.01 C)
    // All other values are invalid values - perhaps 'null'

    // Return 30.00 C as MinConstTemp
    return 3000;
}

int16_t PumpManager::GetMaxConstTemp()
{
    // 1.6.13. MaxConstTemp Attribute
    // Range -273.15 C to 327.67 C (steps of 0.01 C)
    // All other values are invalid values - perhaps 'null'

    // Return 56.00 C as MaxConstTemp
    return 5600;
}
