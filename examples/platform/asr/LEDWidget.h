/*
 *
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

#include <stdint.h>
#ifdef CFG_PLF_RV32
#include "asr_pwm.h"
#define duet_pwm_dev_t asr_pwm_dev_t
#else
#include "duet_pwm.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif
void RGB_setup_HSB(uint8_t Hue, uint8_t Saturation);
#ifdef __cplusplus
}
#endif

#define LIGHT_SELECT_LED 1
#define LIGHT_SELECT_RGB 2
#define LIGHT_SELECT LIGHT_SELECT_RGB

#define LIGHT_RGB_RED PWM_OUTPUT_CH6
#define LIGHT_RGB_GREEN PWM_OUTPUT_CH4
#define LIGHT_RGB_BLUE PWM_OUTPUT_CH1

#define LIGHT_RGB_RED_PAD PAD7
#define LIGHT_RGB_GREEN_PAD PAD6
#define LIGHT_RGB_BLUE_PAD PAD10

class LEDWidget
{
public:
    static void InitGpio(void);
    void Init(uint8_t port);
    void Set(bool state);
    bool Get(void);
    void Invert(void);
    void Blink(uint32_t changeRateMS);
    void Blink(uint32_t onTimeMS, uint32_t offTimeMS);
    void Animate();
    void PWM_start();
    void PWM_stop();
    void RGB_init();
    void SetBrightness(uint8_t brightness);
    void SetColor(uint8_t Hue, uint8_t Saturation);
    void HSB2rgb(uint16_t Hue, uint8_t Saturation, uint8_t brightness, uint8_t & red, uint8_t & green, uint8_t & blue);
    void showRGB(uint8_t red, uint8_t green, uint8_t blue);

private:
    uint64_t mLastChangeTimeMS = 0;
    uint32_t mBlinkOnTimeMS    = 0;
    uint32_t mBlinkOffTimeMS   = 0;
    bool mState                = 0;
    uint8_t mbrightness        = 0;

    uint8_t mHue;
    uint8_t mSaturation;
    duet_pwm_dev_t pwm_led;
    duet_pwm_dev_t pwm_led_g;
    duet_pwm_dev_t pwm_led_b;
    void DoSet(bool state);
};
