/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
    void SuppressOutput();
    void ApplyLevel();

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
};
