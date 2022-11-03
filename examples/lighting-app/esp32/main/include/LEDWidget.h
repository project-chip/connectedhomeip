/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "driver/gpio.h"
#include "esp_log.h"

#if CONFIG_LED_TYPE_RMT
#include "driver/rmt.h"
#include "led_strip.h"
#else
#include "driver/ledc.h"
#include "hal/ledc_types.h"
#endif

class LEDWidget
{
public:
    void Init(void);
    void Set(bool state);
    void Toggle(void);

    void SetBrightness(uint8_t brightness);
    void UpdateState();
#if CONFIG_LED_TYPE_RMT
    void SetColor(uint8_t Hue, uint8_t Saturation);
#endif
    uint8_t GetLevel(void);
    bool IsTurnedOn(void);

private:
    bool mState;
    uint8_t mBrightness;

#if CONFIG_LED_TYPE_RMT
    uint8_t mHue;
    uint8_t mSaturation;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    led_strip_handle_t mStrip;
#else
    led_strip_t * mStrip;
#endif
#else
    gpio_num_t mGPIONum;
#endif

    void DoSet(void);
};
