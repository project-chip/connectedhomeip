/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(LEDWidget);

static LEDWidget::LEDWidgetStateUpdateHandler sStateUpdateCallback;

void LEDWidget::SetStateUpdateCallback(LEDWidgetStateUpdateHandler stateUpdateCb)
{
    if (stateUpdateCb)
        sStateUpdateCallback = stateUpdateCb;
}

void LEDWidget::Init(gpio_dt_spec gpio)
{
    mBlinkOnTimeMS  = 0;
    mBlinkOffTimeMS = 0;
    mGPIO           = gpio;
    mState          = false;

    if (!gpio_is_ready_dt(&mGPIO))
    {
        LOG_ERR("GPIO device not ready");
    }
    else
    {
        int ret = gpio_pin_configure_dt(&mGPIO, GPIO_OUTPUT_ACTIVE);
        if (ret < 0)
        {
            LOG_ERR("GPIO pin %d configure - fail. Status%d\n", mGPIO.pin, ret);
        }
    }

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
    if (!gpio_is_ready_dt(&mGPIO))
    {
        return;
    }
    mState  = state;
    int ret = gpio_pin_set_dt(&mGPIO, state);
    if (ret < 0)
    {
        LOG_ERR("GPIO pin %d set -fail. Status: %d\n", mGPIO.pin, ret);
    }
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
        sStateUpdateCallback(reinterpret_cast<LEDWidget *>(timer->user_data));
}
