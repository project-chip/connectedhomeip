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

#include <InitPlatform.h>

static void showRGB(uint8_t red, uint8_t green, uint8_t blue)
{
    BL602_LightState_Update(red, green, blue);
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
    log_info("Setting state to %d", state ? 1 : 0);
    if (state == mState)
        return;

    mState         = state;
    mBlinkOnTimeMS = mBlinkOffTimeMS = 0;
    DoSet();
}

void LEDWidget::SetBrightness(uint8_t brightness)
{
    uint8_t red, green, blue;
    log_info("mDefaultOnBrightness: %d, brightness: %d\r\n", mDefaultOnBrightness, brightness);
    HSB2rgb(mHue, mSaturation, brightness, red, green, blue);
    log_info("brightness: %d, mHue: %d, mSaturation: %d, red: %d, green: %d, blue: %d\r\n", brightness, mHue, mSaturation, red,
             green, blue);
    showRGB(red, green, blue);
    mDefaultOnBrightness = brightness;
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

void LEDWidget::DoSet()
{
    uint8_t red, green, blue;
    uint8_t brightness = mState ? mDefaultOnBrightness : 0;
    log_info("state: %d, mDefaultOnBrightness: %d, brightness: %d\r\n", mState, mDefaultOnBrightness, brightness);
    HSB2rgb(mHue, mSaturation, brightness, red, green, blue);
    log_info("brightness: %d, mHue: %d, mSaturation: %d, red: %d, green: %d, blue: %d\r\n", brightness, mHue, mSaturation, red,
             green, blue);
    showRGB(red, green, blue);
}

void LEDWidget::Toggle()
{
    log_info("Toggling state to %d", !mState);
    mState = !mState;
    if (mState == 1)
    {
        SetBrightness(UINT8_MAX);
    }
    else
    {
        SetBrightness(0);
    }
}
uint8_t LEDWidget::GetLevel()
{
    return this->mDefaultOnBrightness;
}

bool LEDWidget::IsTurnedOn()
{
    return this->mState;
}
void LEDWidget::SetColor(uint8_t Hue, uint8_t Saturation)
{
    uint8_t red, green, blue;
    uint8_t brightness = mDefaultOnBrightness;
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
