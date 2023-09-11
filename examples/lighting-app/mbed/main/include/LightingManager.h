/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>

#include "AppEvent.h"

#include "drivers/PwmOut.h"

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

    void Init(PinName pwmPinName);
    bool IsTurnedOn() const { return mState == kState_On; }
    uint8_t GetLevel() const { return mLevel; }
    bool InitiateAction(Action_t aAction, int32_t aActor, uint8_t size, uint8_t * value);
    void SetCallbacks(LightingCallback_fn aActionInitiated_CB, LightingCallback_fn aActionCompleted_CB);

private:
    static constexpr uint8_t kMaxLevel = 255;

    friend LightingManager & LightingMgr();
    State_t mState;
    uint8_t mLevel;
    mbed::PwmOut * mPwmDevice;

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
