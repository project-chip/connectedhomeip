/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
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
    mLastChangeTimeMS = 0;
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
    mLastChangeTimeMS = mBlinkOnTimeMS = mBlinkOffTimeMS = 0;
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
        uint64_t nowMS            = chip::System::SystemClock().GetMonotonicMilliseconds64().count();
        uint64_t stateDurMS       = sl_led_get_state(mLed) ? mBlinkOnTimeMS : mBlinkOffTimeMS;
        uint64_t nextChangeTimeMS = mLastChangeTimeMS + stateDurMS;

        if (nextChangeTimeMS < nowMS)
        {
            Invert();
            mLastChangeTimeMS = nowMS;
        }
    }
}
