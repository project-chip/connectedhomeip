/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "PWMDevice.h"

#include "AppConfig.h"

#include <lib/support/CodeUtils.h>

#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>
#include <zephyr/zephyr.h>

LOG_MODULE_DECLARE(app);

CHIP_ERROR PWMDevice::Init(const pwm_dt_spec * pwmDevice, uint8_t aMinLevel, uint8_t aMaxLevel, uint8_t aDefaultLevel)
{
    // We use a gpioPin instead of a LEDWidget here because we want to use PWM
    // and other features instead of just on/off.

    mState     = kState_On;
    mMinLevel  = aMinLevel;
    mMaxLevel  = aMaxLevel;
    mLevel     = aDefaultLevel;
    mPwmDevice = pwmDevice;

    if (!device_is_ready(mPwmDevice->dev))
    {
        LOG_ERR("PWM device %s is not ready", mPwmDevice->dev->name);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    Set(false);
    return CHIP_NO_ERROR;
}

void PWMDevice::SetCallbacks(PWMCallback_fn aActionInitiated_CB, PWMCallback_fn aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

bool PWMDevice::InitiateAction(Action_t aAction, int32_t aActor, uint8_t * value)
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
    else if ((aAction == LEVEL_ACTION || aAction == COLOR_ACTION_XY || aAction == COLOR_ACTION_HSV || aAction == COLOR_ACTION_CT) &&
             *value != mLevel)
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
        else if (aAction == LEVEL_ACTION || aAction == COLOR_ACTION_XY || aAction == COLOR_ACTION_HSV || aAction == COLOR_ACTION_CT)
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

void PWMDevice::SetLevel(uint8_t aLevel)
{
    LOG_INF("Setting brightness level to %u", aLevel);
    mLevel = aLevel;
    UpdateLight();
}

void PWMDevice::Set(bool aOn)
{
    mState = aOn ? kState_On : kState_Off;
    UpdateLight();
}

void PWMDevice::UpdateLight()
{
    constexpr uint32_t kPwmWidthUs  = 20000u;
    const uint8_t maxEffectiveLevel = mMaxLevel - mMinLevel;
    const uint8_t effectiveLevel    = mState == kState_On ? chip::min<uint8_t>(mLevel - mMinLevel, maxEffectiveLevel) : 0;

    pwm_set(mPwmDevice->dev, mPwmDevice->channel, PWM_USEC(kPwmWidthUs), PWM_USEC(kPwmWidthUs * effectiveLevel / maxEffectiveLevel),
            0);
}
