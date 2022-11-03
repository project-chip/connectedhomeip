/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <FreeRTOS.h>
#include <stdint.h>
#include <task.h>
#include <timers.h>

class LEDWidget
{
public:
    LEDWidget() { Init(0); }

    void Init(uint8_t gpioNum);

    void Set(bool state);

    void SetStateMask(bool state);

    void SetBrightness(uint8_t brightness);

    void Blink(uint32_t changeRateMS);

    void Blink(uint32_t onTimeMS, uint32_t offTimeMS);

    void BlinkOnError();

    void Animate();
    void SetColor(uint8_t Hue, uint8_t Saturation);
    void Toggle();
    uint8_t GetLevel();
    bool IsTurnedOn();
    void HSB2rgb(uint16_t Hue, uint8_t Saturation, uint8_t brightness, uint8_t & red, uint8_t & green, uint8_t & blue);

private:
    int64_t mLastChangeTimeMS;
    uint32_t mBlinkOnTimeMS;
    uint32_t mBlinkOffTimeMS;
    uint8_t mDefaultOnBrightness;
    uint16_t mHue;       // mHue [0, 360]
    uint8_t mSaturation; // mSaturation [0, 100]
    uint8_t mGPIONum;
    int mVLED1;
    int mVLED2;
    bool mState;
    bool mError;
    TimerHandle_t errorTimer;

    void DoSet();

    friend void ClearErrorState(TimerHandle_t);
};
