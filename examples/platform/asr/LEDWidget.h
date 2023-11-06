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
#include "asr_gpio.h"
#include "asr_pinmux.h"
#include "asr_pwm.h"
#define duet_pwm_dev_t asr_pwm_dev_t
#define duet_pwm_config_t asr_pwm_config_t
#define duet_pwm_para_chg asr_pwm_para_chg
#define duet_pwm_init asr_pwm_init
#define duet_pwm_start asr_pwm_start
#define duet_gpio_dev_t asr_gpio_dev_t
#define duet_gpio_init asr_gpio_init
#define duet_gpio_output_low asr_gpio_output_low
#define duet_gpio_output_high asr_gpio_output_high
#define DUET_OUTPUT_PUSH_PULL ASR_OUTPUT_PUSH_PULL
#elif defined CFG_PLF_DUET
#include "duet_gpio.h"
#include "duet_pinmux.h"
#include "duet_pwm.h"
#else
#include "lega_gpio.h"
#include "lega_pinmux.h"
#include "lega_pwm.h"
#define duet_pwm_dev_t lega_pwm_dev_t
#define duet_pwm_config_t lega_pwm_config_t
#define duet_pwm_para_chg lega_pwm_para_chg
#define duet_pwm_init lega_pwm_init
#define duet_pwm_start lega_pwm_start
#define duet_gpio_dev_t lega_gpio_dev_t
#define duet_gpio_init lega_gpio_init
#define duet_gpio_output_low lega_gpio_output_low
#define duet_gpio_output_high lega_gpio_output_high
#define DUET_OUTPUT_PUSH_PULL LEGA_OUTPUT_PUSH_PULL
#endif
#ifdef __cplusplus
extern "C" {
#endif
void RGB_setup_HSB(uint8_t Hue, uint8_t Saturation);
#ifdef __cplusplus
}
#endif

#define LIGHT_RGB_RED PWM_OUTPUT_CH6
#define LIGHT_RGB_GREEN PWM_OUTPUT_CH4
#define LIGHT_RGB_BLUE PWM_OUTPUT_CH1

#define LIGHT_RGB_RED_PAD PAD7
#define LIGHT_RGB_GREEN_PAD PAD6
#define LIGHT_RGB_BLUE_PAD PAD10

#define LIGHT_LED GPIO6_INDEX
#define STATE_LED GPIO7_INDEX

class LEDWidget
{
public:
    void Init(uint8_t port);
    void Set(bool state);
    bool Get(void);
    void Invert(void);
    void Blink(uint32_t changeRateMS);
    void Blink(uint32_t onTimeMS, uint32_t offTimeMS);
    void Animate();
    void PWM_start();
    void PWM_stop();
#ifdef LIGHT_SELECT_RGB
    void RGB_init();
    void SetBrightness(uint8_t brightness);
    void SetColor(uint8_t Hue, uint8_t Saturation);
    void HSB2rgb(uint16_t Hue, uint8_t Saturation, uint8_t brightness, uint8_t & red, uint8_t & green, uint8_t & blue);
    void showRGB(uint8_t red, uint8_t green, uint8_t blue);
#endif
private:
    uint64_t mLastChangeTimeMS = 0;
    uint32_t mBlinkOnTimeMS    = 0;
    uint32_t mBlinkOffTimeMS   = 0;
    bool mState                = 0;
    uint8_t mbrightness        = 0;

    uint8_t mHue;
    uint8_t mSaturation;
#ifdef LIGHT_SELECT_RGB
    duet_pwm_dev_t pwm_led;
    duet_pwm_dev_t pwm_led_g;
    duet_pwm_dev_t pwm_led_b;
#else
    duet_gpio_dev_t gpio;
#endif
    void DoSet(bool state);
};
