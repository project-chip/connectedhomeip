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

#include <zephyr.h>

#define LED_PORT DT_LABEL(DT_NODELABEL(gpiob))

const struct device * LEDWidget::mPort = NULL;

void LEDWidget::InitGpio()
{
    mPort = device_get_binding(LED_PORT);
    __ASSERT(mPort != NULL, "Fail to bind with LED port\n");
}

void LEDWidget::Init(gpio_pin_t gpioNum)
{
    mLastChangeTimeMS = 0;
    mBlinkOnTimeMS    = 0;
    mBlinkOffTimeMS   = 0;
    mGPIONum          = gpioNum;
    mState            = false;

    int ret = gpio_pin_configure(mPort, mGPIONum, GPIO_OUTPUT_ACTIVE);
    __ASSERT(ret >= 0, "GPIO pin %d configure - fail. Status%d\n", mGPIONum, ret);

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
        int64_t nowMS      = k_uptime_get();
        int64_t stateDurMS = mState ? mBlinkOnTimeMS : mBlinkOffTimeMS;

        if (nowMS > mLastChangeTimeMS + stateDurMS)
        {
            DoSet(!mState);
            mLastChangeTimeMS = nowMS;
        }
    }
}

void LEDWidget::DoSet(bool state)
{
    mState  = state;
    int ret = gpio_pin_set(mPort, mGPIONum, state);
    __ASSERT(ret >= 0, "GPIO pin %d set -fail. Status: %d\n", mGPIONum, ret);
}
