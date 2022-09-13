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

#include "OnOffPlugManager.h"

#include "AppConfig.h"
#include "AppTask.h"
#include <FreeRTOS.h>

using namespace chip;
using namespace ::chip::DeviceLayer;

OnOffPlugManager OnOffPlugManager::sPlug;

TimerHandle_t sPlugTimer;

namespace {

/**********************************************************
 * OffWithEffect Callbacks
 *********************************************************/

OnOffEffect gEffect = {
    chip::EndpointId{ 1 },
    PlugMgr().OnTriggerOffWithEffect,
    EMBER_ZCL_ON_OFF_EFFECT_IDENTIFIER_DELAYED_ALL_OFF,
    static_cast<uint8_t>(EMBER_ZCL_ON_OFF_DELAYED_ALL_OFF_EFFECT_VARIANT_FADE_TO_OFF_IN_0P8_SECONDS),
};

} // namespace

CHIP_ERROR OnOffPlugManager::Init()
{
    // Create FreeRTOS sw timer for plug timer.
    sPlugTimer = xTimerCreate("plugTmr",        // Just a text name, not used by the RTOS kernel
                              1,                // == default timer period (mS)
                              false,            // no timer reload (==one-shot)
                              (void *) this,    // init timer id = plug obj context
                              TimerEventHandler // timer callback handler
    );

    if (sPlugTimer == NULL)
    {
        EFR32_LOG("sPlugTimer timer create failed");
        return APP_ERROR_CREATE_TIMER_FAILED;
    }

    bool currentLedState;
    // read current on/off value on endpoint one.
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    OnOffServer::Instance().getOnOffValue(1, &currentLedState);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    mIsOn                = currentLedState;
    mAutoTurnOff         = false;
    mAutoTurnOffDuration = 0;
    mOffEffectArmed      = false;

    return CHIP_NO_ERROR;
}

void OnOffPlugManager::SetCallbacks(Callback_fn_apply aApplyAction_CB)
{
    mApplyAction_CB = aApplyAction_CB;
}

bool OnOffPlugManager::IsPlugOn()
{
    return mIsOn;
}

void OnOffPlugManager::EnableAutoTurnOff(bool aOn)
{
    mAutoTurnOff = aOn;
}

void OnOffPlugManager::SetAutoTurnOffDuration(uint32_t aDurationInSecs)
{
    mAutoTurnOffDuration = aDurationInSecs;
}

bool OnOffPlugManager::InitiateAction(int32_t aActor, Action_t aAction)
{
    bool action_initiated = false;

    // Initiate Turn On/Off Action only when the previous one is complete.
    if (((mIsOn == false) || mOffEffectArmed) && aAction == ON_ACTION)
    {
        action_initiated = true;
        if (mOffEffectArmed)
        {
            CancelTimer();
            mOffEffectArmed = false;
        }
    }
    else if (mIsOn && aAction == OFF_ACTION && mOffEffectArmed == false)
    {
        action_initiated = true;
        if (mAutoTurnOffTimerArmed)
        {
            // If auto turn off timer has been armed and someone initiates turning off,
            // cancel the timer and continue as normal.
            mAutoTurnOffTimerArmed = false;
            CancelTimer();
        }
    }

    if (action_initiated)
    {
        mIsOn = aAction;
        if (mApplyAction_CB)
        {
            mApplyAction_CB(aAction, aActor);
        }

        if (mAutoTurnOff && mIsOn == ON_ACTION)
        {
            // Start the timer for auto turn off
            StartTimer(mAutoTurnOffDuration * 1000);

            mAutoTurnOffTimerArmed = true;

            EFR32_LOG("Auto Turn off enabled. Will be triggered in %u seconds", mAutoTurnOffDuration);
        }
    }

    return action_initiated;
}

void OnOffPlugManager::StartTimer(uint32_t aTimeoutMs)
{
    if (xTimerIsTimerActive(sPlugTimer))
    {
        EFR32_LOG("app timer already started!");
        CancelTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(sPlugTimer, (aTimeoutMs / portTICK_PERIOD_MS), 100) != pdPASS)
    {
        EFR32_LOG("sPlugTimer timer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }
}

void OnOffPlugManager::CancelTimer(void)
{
    if (xTimerStop(sPlugTimer, 0) == pdFAIL)
    {
        EFR32_LOG("sPlugTimer stop() failed");
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }
}

void OnOffPlugManager::TimerEventHandler(TimerHandle_t xTimer)
{
    // Get plug obj context from timer id.
    OnOffPlugManager * plug = static_cast<OnOffPlugManager *>(pvTimerGetTimerID(xTimer));

    // The timer event handler will be called in the context of the timer task
    // once sPlugTimer expires. Post an event to apptask queue with the actual handler
    // so that the event can be handled in the context of the apptask.
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = plug;
    if (plug->mAutoTurnOffTimerArmed)
    {
        event.Handler = AutoTurnOffTimerEventHandler;
    }
    else if (plug->mOffEffectArmed)
    {
        event.Handler = OffEffectTimerEventHandler;
    }
    GetAppTask().PostEvent(&event);
}

void OnOffPlugManager::AutoTurnOffTimerEventHandler(AppEvent * aEvent)
{
    OnOffPlugManager * plug = static_cast<OnOffPlugManager *>(aEvent->TimerEvent.Context);
    int32_t actor           = AppEvent::kEventType_Timer;

    // Make sure auto turn off timer is still armed.
    if (!plug->mAutoTurnOffTimerArmed)
    {
        return;
    }

    plug->mAutoTurnOffTimerArmed = false;

    EFR32_LOG("Auto Turn Off has been triggered!");

    plug->InitiateAction(actor, OFF_ACTION);
}

void OnOffPlugManager::OffEffectTimerEventHandler(AppEvent * aEvent)
{
    OnOffPlugManager * plug = static_cast<OnOffPlugManager *>(aEvent->TimerEvent.Context);
    int32_t actor           = AppEvent::kEventType_Timer;

    // Make sure auto turn off timer is still armed.
    if (!plug->mOffEffectArmed)
    {
        return;
    }

    plug->mOffEffectArmed = false;

    EFR32_LOG("OffEffect completed");

    plug->InitiateAction(actor, OFF_ACTION);
}

void OnOffPlugManager::OnTriggerOffWithEffect(OnOffEffect * effect)
{
    chip::app::Clusters::OnOff::OnOffEffectIdentifier effectId = effect->mEffectIdentifier;
    uint8_t effectVariant                                      = effect->mEffectVariant;
    uint32_t offEffectDuration                                 = 0;

    // Temporary print outs and delay to test OffEffect behaviour
    // Until dimming is supported for dev boards.
    if (effectId == EMBER_ZCL_ON_OFF_EFFECT_IDENTIFIER_DELAYED_ALL_OFF)
    {
        if (effectVariant == EMBER_ZCL_ON_OFF_DELAYED_ALL_OFF_EFFECT_VARIANT_FADE_TO_OFF_IN_0P8_SECONDS)
        {
            offEffectDuration = 800;
            ChipLogProgress(Zcl, "EMBER_ZCL_ON_OFF_DELAYED_ALL_OFF_EFFECT_VARIANT_FADE_TO_OFF_IN_0P8_SECONDS");
        }
        else if (effectVariant == EMBER_ZCL_ON_OFF_DELAYED_ALL_OFF_EFFECT_VARIANT_NO_FADE)
        {
            offEffectDuration = 800;
            ChipLogProgress(Zcl, "EMBER_ZCL_ON_OFF_DELAYED_ALL_OFF_EFFECT_VARIANT_NO_FADE");
        }
        else if (effectVariant ==
                 EMBER_ZCL_ON_OFF_DELAYED_ALL_OFF_EFFECT_VARIANT_50_PERCENT_DIM_DOWN_IN_0P8_SECONDS_THEN_FADE_TO_OFF_IN_12_SECONDS)
        {
            offEffectDuration = 12800;
            ChipLogProgress(Zcl,
                            "EMBER_ZCL_ON_OFF_DELAYED_ALL_OFF_EFFECT_VARIANT_50_PERCENT_DIM_DOWN_IN_0P8_SECONDS_THEN_FADE_TO_OFF_"
                            "IN_12_SECONDS");
        }
    }
    else if (effectId == EMBER_ZCL_ON_OFF_EFFECT_IDENTIFIER_DYING_LIGHT)
    {
        if (effectVariant ==
            EMBER_ZCL_ON_OFF_DYING_LIGHT_EFFECT_VARIANT_20_PERCENTER_DIM_UP_IN_0P5_SECONDS_THEN_FADE_TO_OFF_IN_1_SECOND)
        {
            offEffectDuration = 1500;
            ChipLogProgress(
                Zcl, "EMBER_ZCL_ON_OFF_DYING_LIGHT_EFFECT_VARIANT_20_PERCENTER_DIM_UP_IN_0P5_SECONDS_THEN_FADE_TO_OFF_IN_1_SECOND");
        }
    }

    PlugMgr().mOffEffectArmed = true;
    PlugMgr().StartTimer(offEffectDuration);
}
