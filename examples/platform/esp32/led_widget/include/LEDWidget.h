/*
 *
 *    Copyright (c) 2020-2026 Project CHIP Authors
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

#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#if CONFIG_LED_TYPE_RMT
#include "led_strip.h"
#endif

class LEDWidget
{
public:
    using StateChangeCallback = void (*)(LEDWidget * led, bool state);

    /**
     * Initialize the LED using the Kconfig-configured GPIO pin (CONFIG_LED_GPIO).
     * Uses LED_TYPE_RMT or LED_TYPE_GPIO based on Kconfig selection.
     */
    void Init();

    /**
     * Initialize the LED on a specific GPIO pin.
     * For RMT mode, the GPIO must match the addressable LED pin.
     * For GPIO mode, uses LEDC PWM for brightness control.
     */
    void Init(gpio_num_t gpioNum);

    void Set(bool state);
    void Toggle(void);

    void SetBrightness(uint8_t brightness);
    uint8_t GetLevel(void);
    bool IsTurnedOn(void);

    void Blink(uint32_t changeRateMS);
    void Blink(uint32_t onTimeMS, uint32_t offTimeMS);
    void Animate();

    void SetStateMask(bool state);

    void SetStateChangeCallback(StateChangeCallback cb) { mStateChangeCb = cb; }

#if CONFIG_LED_TYPE_RMT
    void SetColor(uint8_t Hue, uint8_t Saturation);
#endif

private:
    bool mState                        = false;
    uint8_t mBrightness                = UINT8_MAX;
    int64_t mLastChangeTimeUS          = 0;
    uint32_t mBlinkOnTimeMS            = 0;
    uint32_t mBlinkOffTimeMS           = 0;
    StateChangeCallback mStateChangeCb = nullptr;

#if CONFIG_LED_TYPE_RMT
    uint8_t mHue              = 0;
    uint8_t mSaturation       = 0;
    led_strip_handle_t mStrip = NULL;
#else
    gpio_num_t mGPIONum = GPIO_NUM_MAX;
#endif

    void DoSet(bool state);
    void UpdateLED(void);
};
