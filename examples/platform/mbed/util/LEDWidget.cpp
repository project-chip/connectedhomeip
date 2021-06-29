/*
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <system/SystemClock.h>

#ifdef MBED_CONF_APP_LED_ACTIVE_STATE
#define LED_ACTIVE_STATE (MBED_CONF_APP_LED_ACTIVE_STATE)
#else
#define LED_ACTIVE_STATE 0
#endif

LEDWidget::LEDWidget(PinName pin) : mLED(pin, !LED_ACTIVE_STATE), mLastChangeTimeMS(0), mBlinkOnTimeMS(0), mBlinkOffTimeMS(0) {}

void LEDWidget::Invert(void)
{
    Set(!mLED);
}

void LEDWidget::Set(bool state)
{
    mBlinkOnTimeMS    = 0;
    mBlinkOffTimeMS   = 0;
    mLastChangeTimeMS = 0;
    mLED              = state ? LED_ACTIVE_STATE : !LED_ACTIVE_STATE;
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
        uint64_t nowMS      = chip::System::Platform::Layer::GetClock_MonotonicMS();
        uint32_t stateDurMS = (mLED == LED_ACTIVE_STATE) ? mBlinkOnTimeMS : mBlinkOffTimeMS;

        if (nowMS > mLastChangeTimeMS + stateDurMS)
        {
            mLED              = !mLED;
            mLastChangeTimeMS = nowMS;
        }
    }
}
