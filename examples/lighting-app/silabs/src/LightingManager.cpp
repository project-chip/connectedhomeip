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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <clusters/ColorControl/AttributeIds.h>
#include <clusters/LevelControl/AttributeIds.h>
#include <lib/support/TypeTraits.h>

using namespace chip;
using namespace ::chip::app;
using namespace ::chip::app::Clusters;
using namespace ::chip::app::Clusters::OnOff;
using namespace ::chip::DeviceLayer;

LightingManager LightingManager::sLight;
namespace {

/**********************************************************
 * OffWithEffect Callbacks
 *********************************************************/

OnOffEffect gEffect = {
    chip::EndpointId{ 1 },
    LightMgr().OnTriggerOffWithEffect,
    EffectIdentifierEnum::kDelayedAllOff,
    to_underlying(DelayedAllOffEffectVariantEnum::kDelayedOffFastFade),
};

} // namespace

CHIP_ERROR LightingManager::Init()
{
    // Create cmsis os sw timer for light timer.
    mLightTimer = osTimerNew(TimerEventHandler, // timer callback handler
                             osTimerOnce,       // no timer reload (one-shot timer)
                             (void *) this,     // pass the app task obj context
                             NULL               // No osTimerAttr_t to provide.
    );

    if (mLightTimer == NULL)
    {
        SILABS_LOG("mLightTimer timer create failed");
        return APP_ERROR_CREATE_TIMER_FAILED;
    }

    bool currentLedState;

    // read current on/off value on endpoint one.
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    OnOffServer::Instance().getOnOffValue(1, &currentLedState);

#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
    app::DataModel::Nullable<uint8_t> brightness;
    uint16_t currentx, currenty, currentctmireds;
    uint8_t currenthue, currentsaturation;

    // Read brightness value
    if (Clusters::LevelControl::Attributes::CurrentLevel::Get(1, brightness) == Protocols::InteractionModel::Status::Success &&
        !brightness.IsNull())
    {
        mCurrentLevel = brightness.Value();
    }

    if (Clusters::ColorControl::Attributes::CurrentX::Get(1, &currentx) == Protocols::InteractionModel::Status::Success)
    {
        mCurrentX = currentx;
    }
    if (Clusters::ColorControl::Attributes::CurrentY::Get(1, &currenty) == Protocols::InteractionModel::Status::Success)
    {
        mCurrentY = currenty;
    }
    if (Clusters::ColorControl::Attributes::CurrentHue::Get(1, &currenthue) == Protocols::InteractionModel::Status::Success)
    {
        mCurrentHue = currenthue;
    }
    if (Clusters::ColorControl::Attributes::CurrentSaturation::Get(1, &currentsaturation) ==
        Protocols::InteractionModel::Status::Success)
    {
        mCurrentSaturation = currentsaturation;
    }
    if (Clusters::ColorControl::Attributes::ColorTemperatureMireds::Get(1, &currentctmireds) ==
        Protocols::InteractionModel::Status::Success)
    {
        mCurrentCTMireds = currentctmireds;
    }
#endif // (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)

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

bool LightingManager::InitiateAction(int32_t aActor, Action_t aAction, uint8_t * aValue)
{
    bool action_initiated = false;
    State_t new_state;

    // Initiate Turn On/Off Action only when the previous one is complete.
    if (((mState == kState_OffCompleted) || mOffEffectArmed) && aAction == ON_ACTION)
    {
        action_initiated = true;

        new_state = kState_OnInitiated;
        if (mOffEffectArmed)
        {
            CancelTimer();
            mOffEffectArmed = false;
        }
    }
    else if (mState == kState_OnCompleted && aAction == OFF_ACTION && mOffEffectArmed == false)
    {
        action_initiated = true;

        new_state = kState_OffInitiated;
        if (mAutoTurnOffTimerArmed)
        {
            // If auto turn off timer has been armed and someone initiates turning off,
            // cancel the timer and continue as normal.
            mAutoTurnOffTimerArmed = false;

            CancelTimer();
        }
    }
    else if (aAction == LEVEL_ACTION)
    {
        action_initiated = true;
    }

    if (action_initiated && (aAction == ON_ACTION || aAction == OFF_ACTION))
    {
        StartTimer(ACTUATOR_MOVEMENT_PERIOS_MS);
        mState = new_state;
    }

    if (action_initiated && mActionInitiated_CB)
    {
        mActionInitiated_CB(aAction, aActor, aValue);
    }

    return action_initiated;
}

void LightingManager::StartTimer(uint32_t aTimeoutMs)
{
    // Starts or restarts the function timer
    if (osTimerStart(mLightTimer, pdMS_TO_TICKS(aTimeoutMs)) != osOK)
    {
        SILABS_LOG("mLightTimer timer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }
}

void LightingManager::CancelTimer(void)
{
    if (osTimerStop(mLightTimer) == osError)
    {
        SILABS_LOG("mLightTimer stop() failed");
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }
}

void LightingManager::TimerEventHandler(void * timerCbArg)
{
    // The callback argument is the light obj context assigned at timer creation.
    LightingManager * light = static_cast<LightingManager *>(timerCbArg);

    // The timer event handler will be called in the context of the timer task
    // once mLightTimer expires. Post an event to apptask queue with the actual handler
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
    uint8_t value           = aEvent->LightEvent.Value;

    // Make sure auto turn off timer is still armed.
    if (!light->mAutoTurnOffTimerArmed)
    {
        return;
    }

    light->mAutoTurnOffTimerArmed = false;

    SILABS_LOG("Auto Turn Off has been triggered!");

    light->InitiateAction(actor, OFF_ACTION, &value);
}

void LightingManager::OffEffectTimerEventHandler(AppEvent * aEvent)
{
    LightingManager * light = static_cast<LightingManager *>(aEvent->TimerEvent.Context);
    int32_t actor           = AppEvent::kEventType_Timer;
    uint8_t value           = aEvent->LightEvent.Value;

    // Make sure auto turn off timer is still armed.
    if (!light->mOffEffectArmed)
    {
        return;
    }

    light->mOffEffectArmed = false;

    SILABS_LOG("OffEffect completed");

    light->InitiateAction(actor, OFF_ACTION, &value);
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
    if (effectId == EffectIdentifierEnum::kDelayedAllOff)
    {
        auto typedEffectVariant = static_cast<DelayedAllOffEffectVariantEnum>(effectVariant);
        if (typedEffectVariant == DelayedAllOffEffectVariantEnum::kDelayedOffFastFade)
        {
            offEffectDuration = 800;
            ChipLogProgress(Zcl, "DelayedAllOffEffectVariantEnum::kDelayedOffFastFade");
        }
        else if (typedEffectVariant == DelayedAllOffEffectVariantEnum::kNoFade)
        {
            offEffectDuration = 800;
            ChipLogProgress(Zcl, "DelayedAllOffEffectVariantEnum::kNoFade");
        }
        else if (typedEffectVariant == DelayedAllOffEffectVariantEnum::kDelayedOffSlowFade)
        {
            offEffectDuration = 12800;
            ChipLogProgress(Zcl, "DelayedAllOffEffectVariantEnum::kDelayedOffSlowFade");
        }
    }
    else if (effectId == EffectIdentifierEnum::kDyingLight)
    {
        auto typedEffectVariant = static_cast<DyingLightEffectVariantEnum>(effectVariant);
        if (typedEffectVariant == DyingLightEffectVariantEnum::kDyingLightFadeOff)
        {
            offEffectDuration = 1500;
            ChipLogProgress(Zcl, "DyingLightEffectVariantEnum::kDyingLightFadeOff");
        }
    }

    LightMgr().mOffEffectArmed = true;
    LightMgr().StartTimer(offEffectDuration);
}

#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
bool LightingManager::InitiateLightCtrlAction(int32_t aActor, Action_t aAction, uint32_t aAttributeId, uint8_t * value)
{
    bool action_initiated = false;
    VerifyOrReturnError(aAction == COLOR_ACTION_XY || aAction == COLOR_ACTION_HSV || aAction == COLOR_ACTION_CT, action_initiated);
    RGBLEDWidget::ColorData_t colorData;
    switch (aAction)
    {
    case COLOR_ACTION_XY:
        colorData.xy = { mCurrentX, mCurrentY };

        if (aAttributeId == ColorControl::Attributes::CurrentX::Id)
        {
            VerifyOrReturnValue(colorData.xy.y != *reinterpret_cast<uint16_t *>(value), action_initiated);
            colorData.xy.y   = *reinterpret_cast<uint16_t *>(value);
            action_initiated = true;
        }
        else if (aAttributeId == ColorControl::Attributes::CurrentY::Id)
        {
            VerifyOrReturnValue(colorData.xy.y != *reinterpret_cast<uint16_t *>(value), action_initiated);
            colorData.xy.y   = *reinterpret_cast<uint16_t *>(value);
            action_initiated = true;
        }
        break;

    case COLOR_ACTION_HSV:
        colorData.hsv = { mCurrentHue, mCurrentSaturation };

        if (aAttributeId == ColorControl::Attributes::CurrentHue::Id)
        {
            VerifyOrReturnValue(colorData.hsv.h != *value, action_initiated);
            colorData.hsv.h  = *value;
            action_initiated = true;
        }
        else if (aAttributeId == ColorControl::Attributes::CurrentSaturation::Id)
        {
            VerifyOrReturnValue(colorData.hsv.s != *value, action_initiated);
            colorData.hsv.s  = *value;
            action_initiated = true;
        }
        break;

    case COLOR_ACTION_CT:
        colorData.ct.ctMireds = mCurrentCTMireds;
        VerifyOrReturnValue(colorData.ct.ctMireds != *(uint16_t *) value, action_initiated);
        colorData.ct.ctMireds = *(uint16_t *) value;
        action_initiated      = true;
        break;

    default:
        break;
    }
    AppTask::GetAppTask().PostLightControlActionRequest(aActor, aAction, (RGBLEDWidget::ColorData_t *) &colorData);
    return action_initiated;
}
#endif // (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
