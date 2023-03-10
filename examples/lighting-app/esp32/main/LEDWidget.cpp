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

#include "LEDWidget.h"
#include "ColorFormat.h"
#if CONFIG_HAVE_DISPLAY
#include "ScreenManager.h"
#endif
#include "led_strip.h"

static const char * TAG = "LEDWidget";

void LEDWidget::Init(void)
{
    mState      = false;
    mBrightness = UINT8_MAX;

#if CONFIG_LED_TYPE_RMT
    rmt_config_t config             = RMT_DEFAULT_CONFIG_TX((gpio_num_t) CONFIG_LED_GPIO, (rmt_channel_t) CONFIG_LED_RMT_CHANNEL);
    led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(1, (led_strip_dev_t) config.channel);

    config.clk_div = 2;
    rmt_config(&config);
    rmt_driver_install(config.channel, 0, 0);

    mStrip      = led_strip_new_rmt_ws2812(&strip_config);
    mHue        = 0;
    mSaturation = 0;
#else
    mGPIONum                       = (gpio_num_t) CONFIG_LED_GPIO;
    ledc_timer_config_t ledc_timer = {
        .speed_mode      = LEDC_LOW_SPEED_MODE, // timer mode
        .duty_resolution = LEDC_TIMER_8_BIT,    // resolution of PWM duty
        .timer_num       = LEDC_TIMER_1,        // timer index
        .freq_hz         = 5000,                // frequency of PWM signal
        .clk_cfg         = LEDC_AUTO_CLK,       // Auto select the source clock
    };
    ledc_timer_config(&ledc_timer);
    ledc_channel_config_t ledc_channel = {
        .gpio_num   = mGPIONum,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .intr_type  = LEDC_INTR_DISABLE,
        .timer_sel  = LEDC_TIMER_1,
        .duty       = 0,
        .hpoint     = 0,
    };
    ledc_channel_config(&ledc_channel);
#endif // CONFIG_LED_TYPE_RMT
}

void LEDWidget::Set(bool state)
{
    ESP_LOGI(TAG, "Setting state to %d", state ? 1 : 0);
    if (state == mState)
        return;

    mState = state;

    DoSet();
}

void LEDWidget::Toggle()
{
    ESP_LOGI(TAG, "Toggling state to %d", !mState);
    mState = !mState;

    DoSet();
}

void LEDWidget::SetBrightness(uint8_t brightness)
{
    ESP_LOGI(TAG, "Setting brightness to %d", brightness);
    if (brightness == mBrightness)
        return;

    mBrightness = brightness;

    DoSet();
}

uint8_t LEDWidget::GetLevel()
{
    return this->mBrightness;
}

bool LEDWidget::IsTurnedOn()
{
    return this->mState;
}

#if CONFIG_LED_TYPE_RMT
void LEDWidget::SetColor(uint8_t Hue, uint8_t Saturation)
{
    if (Hue == mHue && Saturation == mSaturation)
        return;

    mHue        = Hue;
    mSaturation = Saturation;

    DoSet();
}
#endif // CONFIG_LED_TYPE_RMT

void LEDWidget::DoSet(void)
{
    uint8_t brightness = mState ? mBrightness : 0;

#if CONFIG_LED_TYPE_RMT
    if (mStrip)
    {
        HsvColor_t hsv = { mHue, mSaturation, brightness };
        RgbColor_t rgb = HsvToRgb(hsv);
        mStrip->set_pixel(mStrip, 0, rgb.r, rgb.g, rgb.b);
        mStrip->refresh(mStrip, 100);
    }
#else
    if (mGPIONum < GPIO_NUM_MAX)
    {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, brightness);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    }
#endif // CONFIG_LED_TYPE_RMT
#if CONFIG_HAVE_DISPLAY
    if (mVirtualLEDIndex != -1)
    {
        ScreenManager::SetVLED(mVirtualLEDIndex, mState);
    }
#endif // CONFIG_HAVE_DISPLAY
}

#if CONFIG_DEVICE_TYPE_M5STACK
void LEDWidget::SetVLED(int id1)
{
    mVirtualLEDIndex = id1;
    if (mVirtualLEDIndex != -1)
    {
        ScreenManager::SetVLED(mVirtualLEDIndex, mState);
    }
}
#endif
