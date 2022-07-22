/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
