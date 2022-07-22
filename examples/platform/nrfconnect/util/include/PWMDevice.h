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

#pragma once

#include <cstdint>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>

class PWMDevice
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

    using PWMCallback = void (*)(Action_t, int32_t);

    int Init(const pwm_dt_spec * aPWMDevice, uint8_t aMinLevel, uint8_t aMaxLevel, uint8_t aDefaultLevel = 0);
    bool IsTurnedOn() const { return mState == kState_On; }
    uint8_t GetLevel() const { return mLevel; }
    uint8_t GetMinLevel() const { return mMinLevel; }
    uint8_t GetMaxLevel() const { return mMaxLevel; }
    bool InitiateAction(Action_t aAction, int32_t aActor, uint8_t * aValue);
    void SetCallbacks(PWMCallback aActionInitiatedClb, PWMCallback aActionCompletedClb);
    const device * GetDevice() { return mPwmDevice->dev; }

private:
    State_t mState;
    uint8_t mMinLevel;
    uint8_t mMaxLevel;
    uint8_t mLevel;
    const pwm_dt_spec * mPwmDevice;
    uint32_t mPwmChannel;

    PWMCallback mActionInitiatedClb;
    PWMCallback mActionCompletedClb;

    void Set(bool aOn);
    void SetLevel(uint8_t aLevel);
    void UpdateLight();
};
