/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "Display.h"

#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#define STATUS_LED_GPIO_NUM ((gpio_num_t) CONFIG_STATUS_LED_GPIO_NUM)

class LEDWidget
{
public:
    void Init(gpio_num_t gpioNum);

    void Set(bool state);

    void SetStateMask(bool state);

    void SetBrightness(uint8_t brightness);

    void Blink(uint32_t changeRateMS);

    void Blink(uint32_t onTimeMS, uint32_t offTimeMS);

    void BlinkOnError();

    void Animate();
#if CONFIG_DEVICE_TYPE_ESP32_C3_DEVKITM
    void SetColor(uint8_t Hue, uint8_t Saturation);

    void HSB2rgb(uint16_t Hue, uint8_t Saturation, uint8_t brightness, uint8_t & red, uint8_t & green, uint8_t & blue);
#endif
#if CONFIG_HAVE_DISPLAY
    void SetVLED(int id1, int id2);
#endif

private:
    int64_t mLastChangeTimeUS;
    uint32_t mBlinkOnTimeMS;
    uint32_t mBlinkOffTimeMS;
    uint8_t mDefaultOnBrightness;
#if CONFIG_DEVICE_TYPE_ESP32_C3_DEVKITM
    uint16_t mHue;       // mHue [0, 360]
    uint8_t mSaturation; // mSaturation [0, 100]
#endif
    gpio_num_t mGPIONum;
    int mVLED1;
    int mVLED2;
    bool mState;
    bool mError;
    TimerHandle_t errorTimer;

    void DoSet(bool state);

    friend void ClearErrorState(TimerHandle_t);
};
