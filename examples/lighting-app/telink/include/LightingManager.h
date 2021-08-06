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

#pragma once

#include "AppEvent.h"

#include <system/SystemError.h>

#include <cstdint>
#include <drivers/gpio.h>

class LightingManager
{
public:
    enum Action_t : uint8_t
    {
        ON_ACTION = 0,
        OFF_ACTION,
        LEVEL_ACTION,

        INVALID_ACTION
    };

    enum State_t : uint8_t
    {
        kState_On = 0,
        kState_Off,
    };

    using LightingCallback_fn = void (*)(Action_t, int32_t);

    CHIP_ERROR Init(const char * pwmDeviceName, uint32_t pwmChannel);
    bool IsTurnedOn() const { return mState == kState_On; }
    uint8_t GetLevel() const { return mLevel; }
    bool InitiateAction(Action_t aAction, int32_t aActor, uint8_t size, uint8_t * value);
    void SetCallbacks(LightingCallback_fn aActionInitiated_CB, LightingCallback_fn aActionCompleted_CB);

private:
    static constexpr uint8_t kMaxLevel = 255;

    friend LightingManager & LightingMgr();
    State_t mState;
    uint8_t mLevel;
    const device * mPwmDevice;
    uint32_t mPwmChannel;

    LightingCallback_fn mActionInitiated_CB;
    LightingCallback_fn mActionCompleted_CB;

    void Set(bool aOn);
    void SetLevel(uint8_t aLevel);
    void UpdateLight();

    static LightingManager sLight;
};

inline LightingManager & LightingMgr(void)
{
    return LightingManager::sLight;
}
