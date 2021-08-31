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

#include <system/SystemClock.h>

void LEDWidget::Init(LED_t led)
{
    mLastChangeTimeUS = 0;
    mBlinkOnTimeMS    = 0;
    mBlinkOffTimeMS   = 0;
    mGPIONum          = led;
    mState            = false;

    Set(false);
}

void LEDWidget::Invert(void)
{
    Set(!mState);
}

void LEDWidget::Set(bool state)
{
    mLastChangeTimeUS = mBlinkOnTimeMS = mBlinkOffTimeMS = 0;
    DoSet(state);
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
        int64_t nowUS = chip::System::Clock::GetMonotonicMicroseconds();
        ;
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
    mState = state;

    if (state)
    {
        LED_TurnOnLed(mGPIONum);
    }
    else
    {
        LED_TurnOffLed(mGPIONum);
    }
}
