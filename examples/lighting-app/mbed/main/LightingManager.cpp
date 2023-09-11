/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "LightingManager.h"
#include "AppTask.h"

#include <lib/support/logging/CHIPLogging.h>

// mbed-os headers
#include "platform/Callback.h"

#ifdef MBED_CONF_APP_LED_ACTIVE_STATE
#define LED_ACTIVE_STATE (MBED_CONF_APP_LED_ACTIVE_STATE)
#else
#define LED_ACTIVE_STATE 0
#endif

LightingManager LightingManager::sLight;

void LightingManager::Init(PinName pwmPinName)
{
    mState     = kState_On;
    mLevel     = kMaxLevel;
    mPwmDevice = new mbed::PwmOut(pwmPinName);

    Set(false);
}

void LightingManager::SetCallbacks(LightingCallback_fn aActionInitiated_CB, LightingCallback_fn aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

bool LightingManager::InitiateAction(Action_t aAction, int32_t aActor, uint8_t size, uint8_t * value)
{
    bool action_initiated = false;
    State_t new_state;

    // Initiate On/Off Action only when the previous one is complete.
    if (mState == kState_Off && aAction == ON_ACTION)
    {
        action_initiated = true;
        new_state        = kState_On;
    }
    else if (mState == kState_On && aAction == OFF_ACTION)
    {
        action_initiated = true;
        new_state        = kState_Off;
    }
    else if (aAction == LEVEL_ACTION && *value != mLevel)
    {
        action_initiated = true;
        if (*value == 0)
        {
            new_state = kState_Off;
        }
        else
        {
            new_state = kState_On;
        }
    }

    if (action_initiated)
    {
        if (mActionInitiated_CB)
        {
            mActionInitiated_CB(aAction, aActor);
        }

        if (aAction == ON_ACTION || aAction == OFF_ACTION)
        {
            Set(new_state == kState_On);
        }
        else if (aAction == LEVEL_ACTION)
        {
            SetLevel(*value);
        }

        if (mActionCompleted_CB)
        {
            mActionCompleted_CB(aAction, aActor);
        }
    }

    return action_initiated;
}

void LightingManager::SetLevel(uint8_t aLevel)
{
    ChipLogProgress(NotSpecified, "Setting brightness level to %u", aLevel);
    mLevel = aLevel;
    UpdateLight();
}

void LightingManager::Set(bool aOn)
{
    mState = aOn ? kState_On : kState_Off;
    UpdateLight();
}

void LightingManager::UpdateLight()
{
    constexpr uint32_t kPwmWidthUs = 20000u;
    const uint8_t level            = mState == kState_On ? mLevel : 0;
    mPwmDevice->period_us(kPwmWidthUs);

    mPwmDevice->write((float) (LED_ACTIVE_STATE ? level : kMaxLevel - level) / kMaxLevel);
}
