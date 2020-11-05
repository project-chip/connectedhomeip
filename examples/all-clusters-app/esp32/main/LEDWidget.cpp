/*
 *
 *    Copyright (c) 2018 Nest Labs, Inc.
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

/**
 * @file LEDWidget.cpp
 *
 * Implements an LED Widget controller that is usually tied to a GPIO
 * It also updates the display widget if it's enabled
 */

#include "LEDWidget.h"

#include "ScreenManager.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"

void LEDWidget::Init(gpio_num_t gpioNum)
{
    mLastChangeTimeUS = 0;
    mBlinkOnTimeMS    = 0;
    mBlinkOffTimeMS   = 0;
    mGPIONum          = gpioNum;
    mVLED1            = -1;
    mVLED2            = -1;
    mState            = false;
    mError            = false;
    errorTimer        = NULL;

    if (gpioNum < GPIO_NUM_MAX)
    {
        gpio_set_direction(gpioNum, GPIO_MODE_OUTPUT);
    }
}

void LEDWidget::Set(bool state)
{
    mBlinkOnTimeMS = mBlinkOffTimeMS = 0;
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

void ClearErrorState(TimerHandle_t handle)
{
#if CONFIG_HAVE_DISPLAY
    LEDWidget * pWidget = (LEDWidget *) pvTimerGetTimerID(handle);
    pWidget->mError     = false;
    if (pWidget->mVLED2 != -1)
    {
        ScreenManager::SetVLED(pWidget->mVLED2, false);
    }
#endif
}

void LEDWidget::BlinkOnError()
{
#if CONFIG_HAVE_DISPLAY
    mError = true;
    if (errorTimer != NULL)
    {
        xTimerDelete(errorTimer, 0);
    }
    errorTimer = xTimerCreate("ErrorTimer", pdMS_TO_TICKS(2000), false, this, ClearErrorState);
    xTimerStart(errorTimer, 0);
    if (mVLED2 != -1)
    {
        ScreenManager::SetVLED(mVLED2, true);
    }
#endif
}

void LEDWidget::Animate()
{
    if (mBlinkOnTimeMS != 0 && mBlinkOffTimeMS != 0)
    {
        int64_t nowUS            = ::esp_timer_get_time();
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
    bool stateChange = (mState != state);
    mState           = state;
    if (mGPIONum < GPIO_NUM_MAX)
    {
        gpio_set_level(mGPIONum, (state) ? 1 : 0);
    }
    if (stateChange)
    {
#if CONFIG_HAVE_DISPLAY
        if (mVLED1 != -1)
        {
            ScreenManager::SetVLED(mVLED1, mState);
        }
#endif
    }
}

#if CONFIG_HAVE_DISPLAY
void LEDWidget::SetVLED(int id1, int id2)
{
    mVLED1 = id1;
    if (mVLED1 != -1)
    {
        ScreenManager::SetVLED(mVLED1, mState);
    }
    mVLED2 = id2;
    if (mVLED2 != -1)
    {
        ScreenManager::SetVLED(mVLED2, mError);
    }
}
#endif
