/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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
