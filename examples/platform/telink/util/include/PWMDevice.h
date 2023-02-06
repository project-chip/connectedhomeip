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

#include <system/SystemError.h>

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
        COLOR_ACTION_XY,
        COLOR_ACTION_HSV,
        COLOR_ACTION_CT,

        INVALID_ACTION
    };

    enum State_t : uint8_t
    {
        kState_On = 0,
        kState_Off,
    };

    enum BreatheType_t : uint8_t
    {
        kBreatheType_Invalid = 0,
        kBreatheType_Rising,
        kBreatheType_Falling,
        kBreatheType_Both,
    };

    using PWMCallback_fn      = void (*)(Action_t, int32_t);
    using PWMTimerCallback_fn = void (*)(k_timer *);

    CHIP_ERROR Init(const pwm_dt_spec * pwmDevice, uint8_t aMinLevel, uint8_t aMaxLevel, uint8_t aDefaultLevel = 0);
    void Set(bool aOn);
    bool IsTurnedOn(void) const { return mState == kState_On; }
    uint8_t GetLevel(void) const { return mLevel; }
    uint8_t GetMinLevel(void) const { return mMinLevel; }
    uint8_t GetMaxLevel(void) const { return mMaxLevel; }
    void SetCallbacks(PWMCallback_fn aActionInitiated_CB, PWMCallback_fn aActionCompleted_CB,
                      PWMTimerCallback_fn aActionBlinkStateUpdate_CB);
    bool InitiateAction(Action_t aAction, int32_t aActor, uint8_t * value);
    void InitiateBlinkAction(uint32_t onTimeMS, uint32_t offTimeMS);
    void InitiateBreatheAction(BreatheType_t type, uint32_t cycleTimeMS);
    void StopAction(void);
    void UpdateAction(void);

private:
    State_t mState;
    uint8_t mMinLevel;
    uint8_t mMaxLevel;
    uint8_t mLevel;
    uint8_t mBreatheStepLevel;
    uint8_t mBreatheStepCntr;
    bool mBreatheBothDirection;
    BreatheType_t mBreatheType;
    uint32_t mBreatheStepNumb;
    uint32_t mBlinkOnTimeMS;
    uint32_t mBlinkOffTimeMS;
    k_timer mPwmLedTimer;
    const pwm_dt_spec * mPwmDevice;

    PWMCallback_fn mActionInitiated_CB;
    PWMCallback_fn mActionCompleted_CB;

    void SetLevel(uint8_t aLevel);
    void UpdateLight(void);
    void StartBlinkTimer(void);
    void StartBreatheTimer(uint32_t stepTimeMS);
    void ClearAction(void);
    static void PwmLedTimerHandler(k_timer * timer);
};
