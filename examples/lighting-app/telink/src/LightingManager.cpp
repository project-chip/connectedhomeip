/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <drivers/pwm.h>
#include <logging/log.h>
#include <zephyr.h>

LOG_MODULE_DECLARE(app);

LightingManager LightingManager::sLight;

int LightingManager::Init(const char * pwmDeviceName, uint32_t pwmChannel)
{
    // We use a gpioPin instead of a LEDWidget here because we want to use PWM
    // and other features instead of just on/off.

    mState      = kState_On;
    mLevel      = kMaxLevel;
    mPwmDevice  = device_get_binding(pwmDeviceName);
    mPwmChannel = pwmChannel;

    if (!mPwmDevice)
    {
        LOG_ERR("Cannot find PWM device %s", log_strdup(pwmDeviceName));
        return -ENODEV;
    }

    Set(false);
    return 0;
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
    LOG_INF("Setting brightness level to %u", aLevel);
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
    pwm_pin_set_usec(mPwmDevice, mPwmChannel, kPwmWidthUs, kPwmWidthUs * level / kMaxLevel, 0);
}
