/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "LEDWidget.h"

#include <platform/CHIPDeviceLayer.h>

void LEDWidget::InitGpio(void)
{
    // Sets gpio pin mode for ALL board Leds.
    // BSP_LedsInit();
    // TODO: 3R
}

void LEDWidget::Init(int ledNum)
{
    mLastChangeTimeUS = 0;
    mBlinkOnTimeMS    = 0;
    mBlinkOffTimeMS   = 0;
    mLedNum           = ledNum;

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
        int64_t nowUS            = ::chip::System::Layer::GetClock_MonotonicHiRes();
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
        // BSP_LedSet(mLedNum);
        // TODO: 3R
    }
    else
    {
        // BSP_LedClear(mLedNum);
        // TODO: 3R
    }
}
