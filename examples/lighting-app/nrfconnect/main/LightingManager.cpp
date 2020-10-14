/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <logging/log.h>
#include <zephyr.h>

LOG_MODULE_DECLARE(app);

LightingManager LightingManager::sLight;

int LightingManager::Init(const char * gpioDeviceName, gpio_pin_t gpioPin)
{
    // We use a gpioPin instead of a LEDWidget here because we want to use PWM
    // and other features instead of just on/off.

    mState      = kState_On;
    mGPIOPin    = gpioPin;
    mGPIODevice = const_cast<device *>(device_get_binding(gpioDeviceName));

    if (!mGPIODevice)
    {
        LOG_ERR("Cannot find GPIO port %s", log_strdup(gpioDeviceName));
        return -ENODEV;
    }

    int res = gpio_pin_configure(mGPIODevice, mGPIOPin, GPIO_OUTPUT);
    if (res != 0)
    {
        LOG_ERR("Cannot configure GPIO pin");
        return res;
    }

    Set(false);
    return 0;
}

void LightingManager::SetCallbacks(LightingCallback_fn aActionInitiated_CB, LightingCallback_fn aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

bool LightingManager::InitiateAction(Action_t aAction)
{
    // TODO: this function is called InitiateAction because we want to implement some features such as ramping up here.
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

    if (action_initiated)
    {
        if (mActionInitiated_CB)
        {
            mActionInitiated_CB(aAction);
        }

        Set(new_state == kState_On);

        if (mActionCompleted_CB)
        {
            mActionCompleted_CB(aAction);
        }
    }

    return action_initiated;
}

void LightingManager::Set(bool aOn)
{
    if (aOn)
    {
        mState = kState_On;
        gpio_pin_set_raw(mGPIODevice, mGPIOPin, 0);
    }
    else
    {
        mState = kState_Off;
        gpio_pin_set_raw(mGPIODevice, mGPIOPin, 1);
    }
}
