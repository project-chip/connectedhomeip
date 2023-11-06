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

#include <dk_buttons_and_leds.h>
#include <zephyr/kernel.h>

static LEDWidget::LEDWidgetStateUpdateHandler sStateUpdateCallback;

void LEDWidget::InitGpio()
{
    dk_leds_init();
}

void LEDWidget::SetStateUpdateCallback(LEDWidgetStateUpdateHandler stateUpdateCb)
{
    if (stateUpdateCb)
        sStateUpdateCallback = stateUpdateCb;
}

void LEDWidget::Init(uint32_t gpioNum)
{
    mBlinkOnTimeMS  = 0;
    mBlinkOffTimeMS = 0;
    mGPIONum        = gpioNum;
    mState          = false;

    k_timer_init(&mLedTimer, &LEDWidget::LedStateTimerHandler, nullptr);
    k_timer_user_data_set(&mLedTimer, this);

    Set(false);
}

void LEDWidget::Invert(void)
{
    Set(!mState);
}

void LEDWidget::Set(bool state)
{
    k_timer_stop(&mLedTimer);
    mBlinkOnTimeMS = mBlinkOffTimeMS = 0;
    DoSet(state);
}

void LEDWidget::Blink(uint32_t changeRateMS)
{
    Blink(changeRateMS, changeRateMS);
}

void LEDWidget::Blink(uint32_t onTimeMS, uint32_t offTimeMS)
{
    k_timer_stop(&mLedTimer);

    mBlinkOnTimeMS  = onTimeMS;
    mBlinkOffTimeMS = offTimeMS;

    if (mBlinkOnTimeMS != 0 && mBlinkOffTimeMS != 0)
    {
        DoSet(!mState);
        ScheduleStateChange();
    }
}

void LEDWidget::ScheduleStateChange()
{
    k_timer_start(&mLedTimer, K_MSEC(mState ? mBlinkOnTimeMS : mBlinkOffTimeMS), K_NO_WAIT);
}

void LEDWidget::DoSet(bool state)
{
    mState = state;
    dk_set_led(mGPIONum, state);
}

void LEDWidget::UpdateState()
{
    /* Prevent from keep updating the state if LED was set to solid On/Off value */
    if (mBlinkOnTimeMS != 0 && mBlinkOffTimeMS != 0)
    {
        DoSet(!mState);
        ScheduleStateChange();
    }
}

void LEDWidget::LedStateTimerHandler(k_timer * timer)
{
    if (sStateUpdateCallback)
        sStateUpdateCallback(*reinterpret_cast<LEDWidget *>(timer->user_data));
}
