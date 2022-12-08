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

#include <cstdint>

extern "C" {
#include <PinNames.h>
#include <hal/gpio_api.h>
#include <mps3_io.h>
}

#include <system/SystemClock.h>

#include <LEDWidget.h>

LEDWidget::LEDWidget(PinName pin)
{
    mps3_io_init(&mLed, &MPS3_IO_DEV_NS, pin);
}

void LEDWidget::Set(bool state)
{
    mBlinkOnTimeMS    = 0;
    mBlinkOffTimeMS   = 0;
    mLastChangeTimeMS = 0;
    mState            = state;
}

void LEDWidget::Invert(void)
{
    Set(!mState);
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
            mdh_gpio_write((mdh_gpio_t *) &mLed, mState);
            mLastChangeTimeMS = nowMS;
        }
    }
}
