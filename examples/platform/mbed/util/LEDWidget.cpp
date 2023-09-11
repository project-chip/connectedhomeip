/*
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
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
        uint64_t nowMS            = chip::System::SystemClock().GetMonotonicMilliseconds64().count();
        uint64_t stateDurMS       = (mLED == LED_ACTIVE_STATE) ? mBlinkOnTimeMS : mBlinkOffTimeMS;
        uint64_t nextChangeTimeMS = mLastChangeTimeMS + stateDurMS;

        if (nextChangeTimeMS < nowMS)
        {
            mLED              = !mLED;
            mLastChangeTimeMS = nowMS;
        }
    }
}
