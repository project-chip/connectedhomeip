/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include "LEDWidget.h"

#include <platform/CHIPDeviceLayer.h>
#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

using namespace ::chip::System;
using namespace chip::DeviceLayer::Silabs;

void LEDWidget::InitGpio(void)
{
    // Sets gpio pin mode for ALL board Leds.
    GetPlatform().InitLed();
}

void LEDWidget::Init(const uint8_t led)
{
    mLastChangeTimeMS = 0;
    mBlinkOnTimeMS    = 0;
    mBlinkOffTimeMS   = 0;
    mLed              = led;
    mLedStatus        = false;

    Set(false);
}

void LEDWidget::Invert(void)
{
    GetPlatform().ToggleLed(mLed);
    mLedStatus = !mLedStatus;
}

void LEDWidget::Set(bool state)
{
    mLastChangeTimeMS = mBlinkOnTimeMS = mBlinkOffTimeMS = 0;
    GetPlatform().SetLed(state, mLed);
    mLedStatus = state;
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

void LEDWidget::Animate()
{
    if (mBlinkOnTimeMS != 0 && mBlinkOffTimeMS != 0)
    {
        uint64_t nowMS            = chip::System::SystemClock().GetMonotonicMilliseconds64().count();
        uint64_t stateDurMS       = mLedStatus ? mBlinkOnTimeMS : mBlinkOffTimeMS;
        uint64_t nextChangeTimeMS = mLastChangeTimeMS + stateDurMS;

        if (nextChangeTimeMS < nowMS)
        {
            Invert();
            mLastChangeTimeMS = nowMS;
        }
    }
}
uint8_t LEDWidget::GetLED()
{
    return mLed;
}
bool LEDWidget::GetLEDStatus(uint8_t led)
{
    return mLedStatus;
}
#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
void RGBLEDWidget::SetColor(uint8_t red, uint8_t blue, uint8_t green)
{
    if (GetLEDStatus(GetLED()))
    {
        GetPlatform().SetLedColor(GetLED(), red, blue, green);
    }
}
void RGBLEDWidget::GetColor(uint16_t r, uint16_t g, uint16_t b)
{
    GetPlatform().GetLedColor(GetLED(), r, g, b);
}
#endif // (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
