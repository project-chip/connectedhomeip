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

#pragma once

#include "AppEvent.h"

#include <cstdint>
#include <drivers/gpio.h>

class LightingManager
{
public:
    enum Action_t : uint8_t
    {
        ON_ACTION = 0,
        OFF_ACTION,

        INVALID_ACTION
    };

    enum State_t : uint8_t
    {
        kState_On = 0,
        kState_Off,
    };

    using LightingCallback_fn = void (*)(Action_t, int32_t);

    int Init(const char * gpioDeviceName, gpio_pin_t gpioPin);
    bool IsTurnedOn() const { return mState == kState_On; }
    bool InitiateAction(Action_t aAction, int32_t aActor);
    void SetCallbacks(LightingCallback_fn aActionInitiated_CB, LightingCallback_fn aActionCompleted_CB);

private:
    friend LightingManager & LightingMgr(void);
    State_t mState;
    gpio_pin_t mGPIOPin;
    device * mGPIODevice;

    LightingCallback_fn mActionInitiated_CB;
    LightingCallback_fn mActionCompleted_CB;

    void Set(bool aOn);

    static LightingManager sLight;
};

inline LightingManager & LightingMgr(void)
{
    return LightingManager::sLight;
}
