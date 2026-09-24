/*
 *
 *    Copyright (c) 2018 Nest Labs, Inc.
 *    Copyright (c) 2026 Project CHIP Authors
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

/**
 * @file LEDWidget.cpp
 *
 * Implements a widget for controlling an LED, including brightness and blinking patterns,
 * with optional support for RGB color (if using RMT-based LED control).
 */

#include "LEDWidget.h"
#include "esp_log.h"
#include "esp_timer.h"

#if !CONFIG_LED_TYPE_RMT
#include "driver/ledc.h"
#include "hal/ledc_types.h"
#endif

static constexpr char TAG[] = "LEDWidget";

void LEDWidget::Init()
{
    Init(static_cast<gpio_num_t>(CONFIG_LED_GPIO));
}

void LEDWidget::Init(gpio_num_t gpioNum)
{
    mState            = false;
    mBrightness       = UINT8_MAX;
    mLastChangeTimeUS = 0;
    mBlinkOnTimeMS    = 0;
    mBlinkOffTimeMS   = 0;

#if CONFIG_LED_TYPE_RMT
    mHue        = 0;
    mSaturation = 0;
    mStrip      = NULL;

    if (gpioNum < GPIO_NUM_MAX)
    {
        led_strip_config_t strip_config = {};
        strip_config.strip_gpio_num     = gpioNum;
        strip_config.max_leds           = 1;

        led_strip_rmt_config_t rmt_config = {};
        rmt_config.resolution_hz          = 10 * 1000 * 1000; // 10 MHz

        if (led_strip_new_rmt_device(&strip_config, &rmt_config, &mStrip) != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to create LED strip on GPIO %d", gpioNum);
            return;
        }
    }
#else
    mGPIONum = gpioNum;
    if (gpioNum < GPIO_NUM_MAX)
    {
        ledc_timer_config_t ledc_timer = {
            .speed_mode      = LEDC_LOW_SPEED_MODE,
            .duty_resolution = LEDC_TIMER_8_BIT,
            .timer_num       = LEDC_TIMER_1,
            .freq_hz         = 5000,
            .clk_cfg         = LEDC_AUTO_CLK,
        };
        ledc_timer_config(&ledc_timer);

        ledc_channel_config_t ledc_channel = {
            .gpio_num   = gpioNum,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel    = LEDC_CHANNEL_0,
            .intr_type  = LEDC_INTR_DISABLE,
            .timer_sel  = LEDC_TIMER_1,
            .duty       = 0,
            .hpoint     = 0,
        };
        ledc_channel_config(&ledc_channel);
    }
#endif // CONFIG_LED_TYPE_RMT

    ESP_LOGI(TAG, "Initialized on GPIO %d", gpioNum);
}

void LEDWidget::Set(bool state)
{
    ESP_LOGI(TAG, "Setting state to %d", state ? 1 : 0);
    mBlinkOnTimeMS = mBlinkOffTimeMS = 0;
    DoSet(state);
}

void LEDWidget::Toggle(void)
{
    ESP_LOGI(TAG, "Toggling state to %d", !mState);
    mBlinkOnTimeMS = mBlinkOffTimeMS = 0;
    DoSet(!mState);
}

void LEDWidget::SetBrightness(uint8_t brightness)
{
    ESP_LOGI(TAG, "Setting brightness to %d", brightness);
    if (brightness == mBrightness)
        return;

    mBrightness = brightness;
    UpdateLED();
}

uint8_t LEDWidget::GetLevel(void)
{
    return mBrightness;
}

bool LEDWidget::IsTurnedOn(void)
{
    return mState;
}

void LEDWidget::Blink(uint32_t changeRateMS)
{
    Blink(changeRateMS, changeRateMS);
}

void LEDWidget::Blink(uint32_t onTimeMS, uint32_t offTimeMS)
{
    mBlinkOnTimeMS    = onTimeMS;
    mBlinkOffTimeMS   = offTimeMS;
    mLastChangeTimeUS = ::esp_timer_get_time();
    Animate();
}

void LEDWidget::Animate()
{
    if (mBlinkOnTimeMS != 0 && mBlinkOffTimeMS != 0)
    {
        int64_t nowUS            = ::esp_timer_get_time();
        int64_t stateDurUS       = ((mState) ? mBlinkOnTimeMS : mBlinkOffTimeMS) * 1000LL;
        int64_t nextChangeTimeUS = mLastChangeTimeUS + stateDurUS;

        if (nowUS > nextChangeTimeUS)
        {
            DoSet(!mState);
            mLastChangeTimeUS = nowUS;
        }
    }
}

void LEDWidget::SetStateMask(bool state)
{
    DoSet(state);
}

#if CONFIG_LED_TYPE_RMT
void LEDWidget::SetColor(uint8_t Hue, uint8_t Saturation)
{
    if (Hue == mHue && Saturation == mSaturation)
        return;

    mHue        = Hue;
    mSaturation = Saturation;

    UpdateLED();
}
#endif // CONFIG_LED_TYPE_RMT

void LEDWidget::DoSet(bool state)
{
    bool stateChange = (mState != state);
    mState           = state;
    UpdateLED();

    if (stateChange && mStateChangeCb)
    {
        mStateChangeCb(this, mState);
    }
}

#if CONFIG_LED_TYPE_RMT
static void HsvToRgb(uint8_t hue, uint8_t saturation, uint8_t value, uint8_t & red, uint8_t & green, uint8_t & blue)
{
    if (saturation == 0)
    {
        red = green = blue = value;
        return;
    }

    uint8_t region    = hue / 43;
    uint8_t remainder = (hue - (region * 43)) * 6;
    uint8_t p         = (value * (255 - saturation)) >> 8;
    uint8_t q         = (value * (255 - ((saturation * remainder) >> 8))) >> 8;
    uint8_t t         = (value * (255 - ((saturation * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
    case 0:
        red = value, green = t, blue = p;
        break;
    case 1:
        red = q, green = value, blue = p;
        break;
    case 2:
        red = p, green = value, blue = t;
        break;
    case 3:
        red = p, green = q, blue = value;
        break;
    case 4:
        red = t, green = p, blue = value;
        break;
    default:
        red = value, green = p, blue = q;
        break;
    }
}
#endif // CONFIG_LED_TYPE_RMT

void LEDWidget::UpdateLED(void)
{
    uint8_t brightness = mState ? mBrightness : 0;

#if CONFIG_LED_TYPE_RMT
    if (mStrip)
    {
        uint8_t red, green, blue;
        HsvToRgb(mHue, mSaturation, brightness, red, green, blue);
        led_strip_set_pixel(mStrip, 0, red, green, blue);
        led_strip_refresh(mStrip);
    }
#else
    if (mGPIONum < GPIO_NUM_MAX)
    {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, brightness);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    }
#endif // CONFIG_LED_TYPE_RMT
}
