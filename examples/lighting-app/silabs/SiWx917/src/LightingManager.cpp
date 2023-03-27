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

#include "LightingManager.h"

#include "AppConfig.h"
#include "AppTask.h"
#include <FreeRTOS.h>

#include <lib/support/TypeTraits.h>

using namespace chip;
using namespace ::chip::app::Clusters::OnOff;
using namespace ::chip::DeviceLayer;

LightingManager LightingManager::sLight;

TimerHandle_t sLightTimer;

namespace {

/**********************************************************
 * OffWithEffect Callbacks
 *********************************************************/

OnOffEffect gEffect = {
    chip::EndpointId{ 1 },
    LightMgr().OnTriggerOffWithEffect,
    OnOffEffectIdentifier::kDelayedAllOff,
    to_underlying(OnOffDelayedAllOffEffectVariant::kFadeToOffIn0p8Seconds),
};

} // namespace

CHIP_ERROR LightingManager::Init()
{
    // Create FreeRTOS sw timer for light timer.
    sLightTimer = xTimerCreate("lightTmr",       // Just a text name, not used by the RTOS kernel
                               pdMS_TO_TICKS(1), // == default timer period
                               false,            // no timer reload (==one-shot)
                               (void *) this,    // init timer id = light obj context
                               TimerEventHandler // timer callback handler
    );

    if (sLightTimer == NULL)
    {
        SILABS_LOG("sLightTimer timer create failed");
        return APP_ERROR_CREATE_TIMER_FAILED;
    }

    bool currentLedState;
    // read current on/off value on endpoint one.
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    OnOffServer::Instance().getOnOffValue(1, &currentLedState);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    mState                 = currentLedState ? kState_OnCompleted : kState_OffCompleted;
    mAutoTurnOffTimerArmed = false;
    mAutoTurnOff           = false;
    mAutoTurnOffDuration   = 0;
    mOffEffectArmed        = false;

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
    if (((mState == kState_OffCompleted) || mOffEffectArmed) && aAction == ON_ACTION)
    {
        action_initiated = true;

        new_state = kState_OnInitiated;
    }
    else if (mState == kState_OnCompleted && aAction == OFF_ACTION && mOffEffectArmed == false)
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

        if (mOffEffectArmed && new_state == kState_OnInitiated)
        {
            CancelTimer();
            mOffEffectArmed = false;
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
        SILABS_LOG("app timer already started!");
        CancelTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ms if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(sLightTimer, pdMS_TO_TICKS(aTimeoutMs), pdMS_TO_TICKS(100)) != pdPASS)
    {
        SILABS_LOG("sLightTimer timer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }
}

void LightingManager::CancelTimer(void)
{
    if (xTimerStop(sLightTimer, pdMS_TO_TICKS(0)) == pdFAIL)
    {
        SILABS_LOG("sLightTimer stop() failed");
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
    else if (light->mOffEffectArmed)
    {
        event.Handler = OffEffectTimerEventHandler;
    }
    else
    {
        event.Handler = ActuatorMovementTimerEventHandler;
    }
    AppTask::GetAppTask().PostEvent(&event);
}

void LightingManager::AutoTurnOffTimerEventHandler(AppEvent * aEvent)
{
    LightingManager * light = static_cast<LightingManager *>(aEvent->TimerEvent.Context);
    int32_t actor           = AppEvent::kEventType_Timer;

    // Make sure auto turn off timer is still armed.
    if (!light->mAutoTurnOffTimerArmed)
    {
        return;
    }

    light->mAutoTurnOffTimerArmed = false;

    SILABS_LOG("Auto Turn Off has been triggered!");

    light->InitiateAction(actor, OFF_ACTION);
}

void LightingManager::OffEffectTimerEventHandler(AppEvent * aEvent)
{
    LightingManager * light = static_cast<LightingManager *>(aEvent->TimerEvent.Context);
    int32_t actor           = AppEvent::kEventType_Timer;

    // Make sure auto turn off timer is still armed.
    if (!light->mOffEffectArmed)
    {
        return;
    }

    light->mOffEffectArmed = false;

    SILABS_LOG("OffEffect completed");

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

            SILABS_LOG("Auto Turn off enabled. Will be triggered in %u seconds", light->mAutoTurnOffDuration);
        }
    }
}

void LightingManager::OnTriggerOffWithEffect(OnOffEffect * effect)
{
    auto effectId              = effect->mEffectIdentifier;
    auto effectVariant         = effect->mEffectVariant;
    uint32_t offEffectDuration = 0;

    // Temporary print outs and delay to test OffEffect behaviour
    // Until dimming is supported for dev boards.
    if (effectId == OnOffEffectIdentifier::kDelayedAllOff)
    {
        auto typedEffectVariant = static_cast<OnOffDelayedAllOffEffectVariant>(effectVariant);
        if (typedEffectVariant == OnOffDelayedAllOffEffectVariant::kFadeToOffIn0p8Seconds)
        {
            offEffectDuration = 800;
            ChipLogProgress(Zcl, "OnOffDelayedAllOffEffectVariant::kFadeToOffIn0p8Seconds");
        }
        else if (typedEffectVariant == OnOffDelayedAllOffEffectVariant::kNoFade)
        {
            offEffectDuration = 800;
            ChipLogProgress(Zcl, "OnOffDelayedAllOffEffectVariant::kNoFade");
        }
        else if (typedEffectVariant == OnOffDelayedAllOffEffectVariant::k50PercentDimDownIn0p8SecondsThenFadeToOffIn12Seconds)
        {
            offEffectDuration = 12800;
            ChipLogProgress(Zcl, "OnOffDelayedAllOffEffectVariant::k50PercentDimDownIn0p8SecondsThenFadeToOffIn12Seconds");
        }
    }
    else if (effectId == OnOffEffectIdentifier::kDyingLight)
    {
        auto typedEffectVariant = static_cast<OnOffDyingLightEffectVariant>(effectVariant);
        if (typedEffectVariant == OnOffDyingLightEffectVariant::k20PercenterDimUpIn0p5SecondsThenFadeToOffIn1Second)
        {
            offEffectDuration = 1500;
            ChipLogProgress(Zcl, "OnOffDyingLightEffectVariant::k20PercenterDimUpIn0p5SecondsThenFadeToOffIn1Second");
        }
    }

    LightMgr().mOffEffectArmed = true;
    LightMgr().StartTimer(offEffectDuration);
}
