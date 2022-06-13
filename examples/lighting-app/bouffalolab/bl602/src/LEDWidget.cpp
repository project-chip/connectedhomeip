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
#include "AppTask.h"
#include <platform/CHIPDeviceLayer.h>

#include <hosal_pwm.h>

#define IOT_DVK_3S 0

#if BOARD_ID == IOT_DVK_3S
hosal_pwm_dev_t pwmR = { .port = 0xff };
#endif

static void showRGB(uint8_t red, uint8_t green, uint8_t blue)
{

#if BOARD_ID == IOT_DVK_3S

    uint32_t level = (red * 10000) / UINT8_MAX;
    log_info("red level: %d\r\n", level);

    if (pwmR.port == 0xff)
    {
        pwmR.port              = 0;
        pwmR.config.pin        = 0;
        pwmR.config.duty_cycle = level; // duty_cycle range is 0~10000 correspond to 0~100%
        pwmR.config.freq       = 1000;
        hosal_pwm_init(&pwmR);
        vTaskDelay(50);
        hosal_pwm_start(&pwmR);
    }
    else
    {
        pwmR.config.duty_cycle = level; // duty_cycle range is 0~10000 correspond to 0~100%
        pwmR.config.freq       = 1000;
        hosal_pwm_para_chg(&pwmR, pwmR.config);
    }
#else

    hosal_pwm_dev_t pwmR, pwmG, pwmB;

    uint32_t level = (red * 10000) / UINT8_MAX;
    log_info("red level: %d\r\n", level);
    pwmR.port              = 0;
    pwmR.config.pin        = 20;
    pwmR.config.duty_cycle = level; // duty_cycle range is 0~10000 correspond to 0~100%
    pwmR.config.freq       = 1000;
    hosal_pwm_init(&pwmR);
    // vTaskDelay(100);

    level = (green * 10000) / UINT8_MAX;
    log_info("green level: %d\r\n", level);
    pwmG.port              = 1;
    pwmG.config.pin        = 21;
    pwmG.config.duty_cycle = level; // duty_cycle range is 0~10000 correspond to 0~100%
    pwmG.config.freq       = 1000;
    hosal_pwm_init(&pwmG);
    // vTaskDelay(100);

    level = (blue * 10000) / UINT8_MAX;
    log_info("blue level: %d\r\n", level);
    pwmB.port = 2;
    // not use debug port
    // pwmB.config.pin        = 17;
    pwmB.config.duty_cycle = level; // duty_cycle range is 0~10000 correspond to 0~100%
    pwmB.config.freq       = 1000;
    hosal_pwm_init(&pwmB);
    vTaskDelay(50);

    hosal_pwm_start(&pwmG);
    hosal_pwm_start(&pwmR);
    hosal_pwm_start(&pwmB);
#endif
}

void LEDWidget::Init(uint8_t gpioNum)
{
    mLastChangeTimeMS = 0;
    mBlinkOnTimeMS    = 0;
    mBlinkOffTimeMS   = 0;
    mGPIONum          = gpioNum;
    mVLED1            = -1;
    mVLED2            = -1;
    mState            = false;
    mError            = false;
    errorTimer        = NULL;

    mDefaultOnBrightness = UINT8_MAX;
    mHue                 = 0;
    mSaturation          = 0;
}

void LEDWidget::Set(bool state)
{
    log_info("state: %d\r\n", state);
    mBlinkOnTimeMS = mBlinkOffTimeMS = 0;
    DoSet(state);
}

void LEDWidget::SetBrightness(uint8_t brightness)
{
    uint8_t red, green, blue;
    log_info("mDefaultOnBrightness: %d, brightness: %d\r\n", mDefaultOnBrightness, brightness);
    HSB2rgb(mHue, mSaturation, brightness, red, green, blue);
    log_info("brightness: %d, mHue: %d, mSaturation: %d, red: %d, green: %d, blue: %d\r\n", brightness, mHue, mSaturation, red,
             green, blue);
    showRGB(red, green, blue);

    if (brightness > 0)
    {
        mDefaultOnBrightness = brightness;
    }
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

void ClearErrorState(TimerHandle_t handle) {}

void LEDWidget::BlinkOnError() {}

void LEDWidget::Animate()
{
    if (mBlinkOnTimeMS != 0 && mBlinkOffTimeMS != 0)
    {
        uint64_t nowMS            = chip::System::SystemClock().GetMonotonicMilliseconds64().count();
        uint64_t stateDurMS       = ((mState) ? mBlinkOnTimeMS : mBlinkOffTimeMS);
        uint64_t nextChangeTimeMS = mLastChangeTimeMS + stateDurMS;

        if (nextChangeTimeMS < nowMS)
        {
            // DoSet(!mState);
            mLastChangeTimeMS = nowMS;
        }
    }
}

void LEDWidget::DoSet(bool state)
{
    bool stateChange = (mState != state);
    mState           = state;

    uint8_t red, green, blue;
    uint8_t brightness = state ? mDefaultOnBrightness : 0;
    log_info("state: %d, mDefaultOnBrightness: %d, brightness: %d\r\n", state, mDefaultOnBrightness, brightness);
    HSB2rgb(mHue, mSaturation, brightness, red, green, blue);
    log_info("brightness: %d, mHue: %d, mSaturation: %d, red: %d, green: %d, blue: %d\r\n", brightness, mHue, mSaturation, red,
             green, blue);
    showRGB(red, green, blue);
}

void LEDWidget::SetColor(uint8_t Hue, uint8_t Saturation)
{
    uint8_t red, green, blue;
    uint8_t brightness = mState ? mDefaultOnBrightness : 0;
    mHue               = static_cast<uint16_t>(Hue) * 360 / 254;        // mHue [0, 360]
    mSaturation        = static_cast<uint16_t>(Saturation) * 100 / 254; // mSaturation [0 , 100]

    HSB2rgb(mHue, mSaturation, brightness, red, green, blue);

    log_info("mDefaultOnBrightness: %d, mHue: %d, mSaturation: %d\r\n", mDefaultOnBrightness, mHue, mSaturation);
    HSB2rgb(mHue, mSaturation, brightness, red, green, blue);
    log_info("brightness: %d, red: %d, green: %d, blue: %d\r\n", brightness, red, green, blue);
    showRGB(red, green, blue);
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
