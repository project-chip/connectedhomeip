/***************************************************************************//**
 * @file
 * @brief Header file for RGB PWM Led controller.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/


#pragma once

#include "LEDWidget.h"

#include "sl_simple_rgb_pwm_led.h"

#define ATTRIBUTE_LEVEL_MAX     254
// Default Saturation in the cluster is zero.
#define INITIAL_SATURATION      0.0f
// Default Hue in the color control cluster is zero.
#define INITIAL_HUE             0
#define INITIAL_RGB             0

#define LED_ENDPOINT_DEFAULT    1
#define LED_RGB_INITIAL_STATE   1
#define LED_RGB_POLARITY        0U
#define PWM_FREQUENCY           10000
#define PWM_MAX_VALUE           100
#define PWM_RESOLUTION_LEVELS   100


// RGB PWM Led instance with preset parameters.
extern const sl_led_rgb_pwm_t sl_led_rgb_pwm;


class LEDWidgetRGB : public LEDWidget
{
public:
    static void InitGpioRGB();
    void Init(const sl_led_rgb_pwm_t* led);
    void Set(bool state);
    void Set(bool state, uint16_t led_endpoint);
    void SetLevel(uint8_t level, uint16_t led_endpoint);
    void GetLevel(uint8_t rgb[], uint32_t size);
    void SetHue(uint8_t hue, uint16_t led_endpoint);
    void SetSaturation(uint8_t sat, uint16_t led_endpoint);
    void SetColor(uint8_t hue, uint8_t saturation, uint16_t led_endpoint);
    void SetColor(uint8_t* rgb, uint16_t led_endpoint);

private:    
    void HueToRGB(uint16_t hue, float saturation, uint8_t* rgb, uint8_t max_value);
    void SetRGB(uint8_t* rgb, uint16_t led_endpoint, bool memorize);
    
    uint16_t current_hue_;
    uint8_t current_rgb_[3];
    float current_saturation_;
    const sl_led_rgb_pwm_t* led_rgb_;
    uint32_t level_resolution_;    
};
