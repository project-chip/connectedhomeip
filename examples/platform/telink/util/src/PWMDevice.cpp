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
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(PWMDevice);

constexpr uint32_t kBreatheStepTimeMS = 10;

static PWMDevice::PWMTimerCallback_fn mActionBlinkStateUpdate_CB;

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
        LOG_ERR("PWM device %s is not ready", mPwmDevice->dev ? mPwmDevice->dev->name : "N/A");
    }

    k_timer_init(&mPwmLedTimer, &PWMDevice::PwmLedTimerHandler, nullptr);
    k_timer_user_data_set(&mPwmLedTimer, this);

    ClearAction();
    Set(false);
    return CHIP_NO_ERROR;
}

void PWMDevice::SetCallbacks(PWMCallback_fn aActionInitiated_CB, PWMCallback_fn aActionCompleted_CB,
                             PWMTimerCallback_fn aActionBlinkStateUpdate_CB)
{
    mActionInitiated_CB        = aActionInitiated_CB;
    mActionCompleted_CB        = aActionCompleted_CB;
    mActionBlinkStateUpdate_CB = aActionBlinkStateUpdate_CB;
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
    LOG_DBG("Setting brightness level to %u", aLevel);
    mLevel = aLevel;
    UpdateLight();
}

void PWMDevice::Set(bool aOn)
{
    mState = aOn ? kState_On : kState_Off;
    UpdateLight();
}

void PWMDevice::UpdateLight(void)
{
    if (!device_is_ready(mPwmDevice->dev))
    {
        return;
    }
    constexpr uint32_t kPwmWidthUs  = 20000u;
    const uint8_t maxEffectiveLevel = mMaxLevel - mMinLevel;
    const uint8_t effectiveLevel    = mState == kState_On ? chip::min<uint8_t>(mLevel - mMinLevel, maxEffectiveLevel) : 0;

    pwm_set(mPwmDevice->dev, mPwmDevice->channel, PWM_USEC(kPwmWidthUs), PWM_USEC(kPwmWidthUs * effectiveLevel / maxEffectiveLevel),
            0);
}

void PWMDevice::InitiateBlinkAction(uint32_t onTimeMS, uint32_t offTimeMS)
{
    ClearAction();

    if (onTimeMS != 0 && offTimeMS != 0)
    {
        mBlinkOnTimeMS  = onTimeMS;
        mBlinkOffTimeMS = offTimeMS;

        Set(mState != kState_On ? true : false);
        StartBlinkTimer();
    }
    else
    {
        LOG_ERR("Invalid InitiateBlinkAction parameters. onTimeMS = %u, offTimeMS = %u", onTimeMS, offTimeMS);
    }
}

void PWMDevice::InitiateBreatheAction(BreatheType_t type, uint32_t cycleTimeMS)
{
    ClearAction();

    if (type != kBreatheType_Invalid && cycleTimeMS != 0)
    {
        mBreatheType      = type;
        mBreatheStepNumb  = cycleTimeMS / kBreatheStepTimeMS;
        mBreatheStepLevel = (mMaxLevel - mMinLevel) / mBreatheStepNumb;

        if (mBreatheType == kBreatheType_Both)
        {
            mBreatheBothDirection = true;
            mBreatheType          = mState == kState_On ? kBreatheType_Falling : kBreatheType_Rising;
        }

        if (mBreatheType == kBreatheType_Falling)
        {
            mLevel = mMaxLevel;
        }
        else
        {
            mLevel = mMinLevel;
        }

        Set(true);
        StartBreatheTimer(kBreatheStepTimeMS);
    }
    else
    {
        LOG_ERR("Invalid InitiateBreatheAction parameters. Type = %u, cycleTimeMS = %u", type, cycleTimeMS);
    }
}

void PWMDevice::StopAction(void)
{
    ClearAction();
    Set(false);
}

void PWMDevice::UpdateAction(void)
{
    // Update of Breathe action
    if (mBreatheType != kBreatheType_Invalid && mBreatheStepLevel != 0 && mBreatheStepNumb != 0)
    {
        if (mBreatheStepCntr == mBreatheStepNumb)
        {
            mBreatheStepCntr = 0;
            if (mBreatheBothDirection)
            {
                mBreatheType = mBreatheType == kBreatheType_Rising ? kBreatheType_Falling : kBreatheType_Rising;
            }
        }
        else
        {
            mBreatheStepCntr++;
        }

        if (mBreatheType == kBreatheType_Rising)
        {
            if (mBreatheStepCntr == mBreatheStepNumb)
            {
                mLevel = mMaxLevel;
            }
            else if (mBreatheStepCntr == 0)
            {
                mLevel = mMinLevel;
            }
            else
            {
                mLevel += mBreatheStepLevel;
            }
        }
        else if (mBreatheType == kBreatheType_Falling)
        {
            if (mBreatheStepCntr == mBreatheStepNumb)
            {
                mLevel = mMinLevel;
            }
            else if (mBreatheStepCntr == 0)
            {
                mLevel = mMaxLevel;
            }
            else
            {
                mLevel -= mBreatheStepLevel;
            }
        }

        Set(true);
        StartBreatheTimer(kBreatheStepTimeMS);
    }
    // Update of Blink action
    else if (mBlinkOnTimeMS != 0 && mBlinkOffTimeMS != 0)
    {
        Set(mState != kState_On ? true : false);
        StartBlinkTimer();
    }
    else
    {
        LOG_ERR("PWM LED update state is incorrect");
    }
}

void PWMDevice::StartBlinkTimer(void)
{
    k_timer_start(&mPwmLedTimer, K_MSEC(mState == kState_On ? mBlinkOnTimeMS : mBlinkOffTimeMS), K_NO_WAIT);
}

void PWMDevice::StartBreatheTimer(uint32_t stepTimeMS)
{
    k_timer_start(&mPwmLedTimer, K_MSEC(stepTimeMS), K_NO_WAIT);
}

void PWMDevice::ClearAction(void)
{
    k_timer_stop(&mPwmLedTimer);
    mBreatheBothDirection = false;
    mBreatheType          = kBreatheType_Invalid;
    mBreatheStepLevel     = 0;
    mBreatheStepNumb      = 0;
    mBlinkOnTimeMS        = 0;
    mBlinkOffTimeMS       = 0;
    mLevel                = mMaxLevel;
}

void PWMDevice::PwmLedTimerHandler(k_timer * timer)
{
    if (mActionBlinkStateUpdate_CB)
    {
        mActionBlinkStateUpdate_CB(timer);
    }
}
