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

#include "app.h"

#if CHIP_CONFIG_ENABLE_DIMMABLE_LED
#include "LED_Dimmer.h"
#endif

#if (defined(gAppLedCnt_c) && (gAppLedCnt_c > 0))

void LEDWidget::Init(uint8_t led, bool inverted)
{
    mLastChangeTimeMS = 0;
    mBlinkOnTimeMS    = 0;
    mBlinkOffTimeMS   = 0;
    mGPIONum          = led;
    mState            = false;
    mOnLogic          = !inverted;

    Set(false);
}

void LEDWidget::Invert(void)
{
    Set(!mState);
}

void LEDWidget::Set(bool state)
{
    mLastChangeTimeMS = mBlinkOnTimeMS = mBlinkOffTimeMS = 0;
    DoSet(state);
}

void LEDWidget::SetLevel(uint8_t level)
{
#if CHIP_CONFIG_ENABLE_DIMMABLE_LED
    move_to_level(level);
#endif
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
        uint64_t stateDurMS       = mState ? mBlinkOnTimeMS : mBlinkOffTimeMS;
        uint64_t nextChangeTimeMS = mLastChangeTimeMS + stateDurMS;

        if (nextChangeTimeMS < nowMS)
        {
#if CHIP_CONFIG_ENABLE_DIMMABLE_LED
            SetLevel(!mState * 254);
            mState = !mState;
#else
            DoSet(!mState);
#endif
            mLastChangeTimeMS = nowMS;
        }
    }
}

void LEDWidget::DoSet(bool state)
{
    mState = state;

    if (state)
    {
        (void) LED_TurnOnOff((led_handle_t) g_ledHandle[mGPIONum], mOnLogic);
    }
    else
    {
        (void) LED_TurnOnOff((led_handle_t) g_ledHandle[mGPIONum], !mOnLogic);
    }
}

#endif /* (defined(gAppLedCnt_c) && (gAppLedCnt_c > 0)) */
