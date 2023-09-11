/*
 *
 * SPDX-FileCopyrightText: 2021-2023 Project CHIP Authors
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

#if CONFIG_DEVICE_TYPE_M5STACK
    // binds this LED to a virtual LED on a screen
    void SetVLED(int id1);
#endif

private:
    bool mState;
    uint8_t mBrightness;

#if CONFIG_LED_TYPE_RMT
    uint8_t mHue;
    uint8_t mSaturation;
    led_strip_t * mStrip;
#else
    gpio_num_t mGPIONum;
#endif

#if CONFIG_DEVICE_TYPE_M5STACK
    int mVirtualLEDIndex = -1;
#endif

    void DoSet(void);
};
