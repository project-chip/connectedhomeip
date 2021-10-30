/*
 *
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

/**
 * @file LEDWidget.cpp
 *
 * Implements an LED Widget controller that is usually tied to a GPIO
 * It also updates the display widget if it's enabled
 */

#include "LEDWidget.h"

#include "ScreenManager.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#if CONFIG_DEVICE_TYPE_ESP32_C3_DEVKITM
#include "driver/rmt.h"
#include "led_strip.h"
#define RMT_TX_DEFAULT_GPIO GPIO_NUM_8
#define RMT_TX_DEFAULT_CHANNEL RMT_CHANNEL_0
static led_strip_t * strip = NULL;
#else
#include "driver/ledc.h"
#include "hal/ledc_types.h"
#endif
void LEDWidget::Init(gpio_num_t gpioNum)
{
    mLastChangeTimeUS = 0;
    mBlinkOnTimeMS    = 0;
    mBlinkOffTimeMS   = 0;
    mGPIONum          = gpioNum;
    mVLED1            = -1;
    mVLED2            = -1;
    mState            = false;
    mError            = false;
    errorTimer        = NULL;
#if CONFIG_DEVICE_TYPE_ESP32_C3_DEVKITM
    if (gpioNum == RMT_TX_DEFAULT_GPIO)
    {
        rmt_config_t config = RMT_DEFAULT_CONFIG_TX(RMT_TX_DEFAULT_GPIO, RMT_TX_DEFAULT_CHANNEL);
        config.clk_div      = 2;
        rmt_config(&config);
        rmt_driver_install(config.channel, 0, 0);
        led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(1, (led_strip_dev_t) config.channel);
        strip                           = led_strip_new_rmt_ws2812(&strip_config);
        mDefaultOnBrightness            = UINT8_MAX;
        mHue                            = 0;
        mSaturation                     = 0;
    }
#else
    if (gpioNum < GPIO_NUM_MAX)
    {
        ledc_timer_config_t ledc_timer = {
            .speed_mode      = LEDC_LOW_SPEED_MODE, // timer mode
            .duty_resolution = LEDC_TIMER_8_BIT,    // resolution of PWM duty
            .timer_num       = LEDC_TIMER_1,        // timer index
            .freq_hz         = 5000,                // frequency of PWM signal
            .clk_cfg         = LEDC_AUTO_CLK,       // Auto select the source clock
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
        mDefaultOnBrightness = UINT8_MAX;
    }
#endif
}

void LEDWidget::Set(bool state)
{
    mBlinkOnTimeMS = mBlinkOffTimeMS = 0;
    DoSet(state);
}

void LEDWidget::SetBrightness(uint8_t brightness)
{
#if CONFIG_DEVICE_TYPE_ESP32_C3_DEVKITM
    if (strip)
    {
        uint8_t red, green, blue;
        HSB2rgb(mHue, mSaturation, brightness, red, green, blue);
        strip->set_pixel(strip, 0, red, green, blue);
        strip->refresh(strip, 100);
    }
#else
    if (mGPIONum < GPIO_NUM_MAX)
    {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, brightness);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    }
#endif
    if (brightness > 0)
    {
        mDefaultOnBrightness = brightness;
    }
#if CONFIG_HAVE_DISPLAY
    if (mVLED1 != -1)
    {
        ScreenManager::SetVLED(mVLED1, mState);
    }
#endif // CONFIG_HAVE_DISPLAY
}

void LEDWidget::Blink(uint32_t changeRateMS)
{
    Blink(changeRateMS, changeRateMS);
}

void LEDWidget::Blink(uint32_t onTimeMS, uint32_t offTimeMS)
{
    mBlinkOnTimeMS  = onTimeMS;
    mBlinkOffTimeMS = offTimeMS;
    Animate();
}

void ClearErrorState(TimerHandle_t handle)
{
#if CONFIG_HAVE_DISPLAY
    LEDWidget * pWidget = (LEDWidget *) pvTimerGetTimerID(handle);
    pWidget->mError     = false;
    if (pWidget->mVLED2 != -1)
    {
        ScreenManager::SetVLED(pWidget->mVLED2, false);
    }
#endif
}

void LEDWidget::BlinkOnError()
{
#if CONFIG_HAVE_DISPLAY
    mError = true;
    if (errorTimer != NULL)
    {
        xTimerDelete(errorTimer, 0);
    }
    errorTimer = xTimerCreate("ErrorTimer", pdMS_TO_TICKS(2000), false, this, ClearErrorState);
    xTimerStart(errorTimer, 0);
    if (mVLED2 != -1)
    {
        ScreenManager::SetVLED(mVLED2, true);
    }
#endif
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

void LEDWidget::DoSet(bool state)
{
    bool stateChange = (mState != state);
    mState           = state;
#if CONFIG_DEVICE_TYPE_ESP32_C3_DEVKITM
    if (strip)
    {
        uint8_t red, green, blue;
        uint8_t brightness = state ? mDefaultOnBrightness : 0;
        HSB2rgb(mHue, mSaturation, brightness, red, green, blue);
        strip->set_pixel(strip, 0, red, green, blue);
        strip->refresh(strip, 100);
    }
#else
    if (mGPIONum < GPIO_NUM_MAX)
    {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, state ? mDefaultOnBrightness : 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    }
#endif
    if (stateChange)
    {
#if CONFIG_HAVE_DISPLAY
        if (mVLED1 != -1)
        {
            ScreenManager::SetVLED(mVLED1, mState);
        }
#endif
    }
}

#if CONFIG_HAVE_DISPLAY
void LEDWidget::SetVLED(int id1, int id2)
{
    mVLED1 = id1;
    if (mVLED1 != -1)
    {
        ScreenManager::SetVLED(mVLED1, mState);
    }
    mVLED2 = id2;
    if (mVLED2 != -1)
    {
        ScreenManager::SetVLED(mVLED2, mError);
    }
}
#endif

#if CONFIG_DEVICE_TYPE_ESP32_C3_DEVKITM
void LEDWidget::SetColor(uint8_t Hue, uint8_t Saturation)
{
    uint8_t red, green, blue;
    uint8_t brightness = mState ? mDefaultOnBrightness : 0;
    mHue               = static_cast<uint16_t>(Hue) * 360 / 254;        // mHue [0, 360]
    mSaturation        = static_cast<uint16_t>(Saturation) * 100 / 254; // mSaturation [0 , 100]

    HSB2rgb(mHue, mSaturation, brightness, red, green, blue);
    strip->set_pixel(strip, 0, red, green, blue);
    strip->refresh(strip, 100);
}

void LEDWidget::HSB2rgb(uint16_t Hue, uint8_t Saturation, uint8_t brightness, uint8_t & red, uint8_t & green, uint8_t & blue)
{
    uint16_t i       = Hue / 60;
    uint16_t rgb_max = brightness;
    uint16_t rgb_min = rgb_max * (100 - Saturation) / 100;
    uint16_t diff    = Hue % 60;
    uint16_t rgb_adj = (rgb_max - rgb_min) * diff / 60;

    switch (i)
    {
    case 0:
        red   = rgb_max;
        green = rgb_min + rgb_adj;
        blue  = rgb_min;
        break;
    case 1:
        red   = rgb_max - rgb_adj;
        green = rgb_max;
        blue  = rgb_min;
        break;
    case 2:
        red   = rgb_min;
        green = rgb_max;
        blue  = rgb_min + rgb_adj;
        break;
    case 3:
        red   = rgb_min;
        green = rgb_max - rgb_adj;
        blue  = rgb_max;
        break;
    case 4:
        red   = rgb_min + rgb_adj;
        green = rgb_min;
        blue  = rgb_max;
        break;
    default:
        red   = rgb_max;
        green = rgb_min;
        blue  = rgb_max - rgb_adj;
        break;
    }
}
#endif
