/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include "cybsp.h"
#include <stdint.h>

class LEDWidget
{
public:
    static void InitGpio(void);
    void Init(int ledNum);
    void Set(bool state);
    bool Get(void);
    void Invert(void);
    void Blink(uint32_t changeRateMS);
    void Blink(uint32_t onTimeMS, uint32_t offTimeMS);
    void Animate();
    void PWM_start();
    void PWM_stop();
    void RGB_init();
    void RGB_set(bool state);
    void SetBrightness(uint32_t led_brightness);
    void SetColor(uint8_t Hue, uint8_t Saturation);
    void HSB2rgb(uint16_t Hue, uint8_t Saturation, uint8_t brightness);

private:
    uint64_t mLastChangeTimeMS = 0;
    uint32_t mBlinkOnTimeMS    = 0;
    uint32_t mBlinkOffTimeMS   = 0;
    int mLedNum                = 0;
    bool mState                = 0;
    uint8_t mbrightness        = 0;
    uint16_t mHue;
    uint8_t mSaturation;
    cyhal_pwm_t pwm_red;
    cyhal_pwm_t pwm_green;
    cyhal_pwm_t pwm_blue;
    cyhal_pwm_t pwm_led;
    void DoSet(bool state);
};
