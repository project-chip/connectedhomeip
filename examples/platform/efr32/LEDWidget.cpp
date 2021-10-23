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
#include "sl_simple_led_instances.h"

#include <platform/CHIPDeviceLayer.h>

using namespace ::chip::System;

void LEDWidget::InitGpio(void)
{
    // Sets gpio pin mode for ALL board Leds.
    sl_simple_led_init_instances();
}

void LEDWidget::Init(const sl_led_t * led)
{
    mLastChangeTimeUS = 0;
    mBlinkOnTimeMS    = 0;
    mBlinkOffTimeMS   = 0;
    mLed              = led;

    Set(false);
}

void LEDWidget::Invert(void)
{
    if (mLed)
    {
        sl_led_toggle(mLed);
    }
}

void LEDWidget::Set(bool state)
{
    mLastChangeTimeUS = mBlinkOnTimeMS = mBlinkOffTimeMS = 0;
    if (mLed)
    {
        state ? sl_led_turn_on(mLed) : sl_led_turn_off(mLed);
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

void LEDWidget::Animate()
{
    if (mBlinkOnTimeMS != 0 && mBlinkOffTimeMS != 0)
    {
        Clock::MonotonicMicroseconds nowUS            = chip::System::SystemClock().GetMonotonicMicroseconds();
        Clock::MonotonicMicroseconds stateDurUS       = ((sl_led_get_state(mLed)) ? mBlinkOnTimeMS : mBlinkOffTimeMS) * 1000LL;
        Clock::MonotonicMicroseconds nextChangeTimeUS = mLastChangeTimeUS + stateDurUS;

        if (nextChangeTimeUS < nowUS)
        {
            Invert();
            mLastChangeTimeUS = nowUS;
        }
    }
}
