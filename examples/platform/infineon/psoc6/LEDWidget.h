/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
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
