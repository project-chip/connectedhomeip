/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "LEDManager.h"
#include "AppTask.h"
#include <platform/CHIPDeviceLayer.h>

#ifdef CFG_PLF_RV32
#define duet_gpio_init asr_gpio_init
#define duet_gpio_output_low asr_gpio_output_low
#define duet_gpio_output_high asr_gpio_output_high
#define DUET_OUTPUT_PUSH_PULL ASR_OUTPUT_PUSH_PULL
#endif

void LEDManager::InitGpio(void) {}

void LEDManager::Init(uint8_t gpioNum)
{
    mLastChangeTimeMS = 0;
    mBlinkOnTimeMS    = 0;
    mBlinkOffTimeMS   = 0;
    mGPIONum          = gpioNum;

    gpio.port   = mGPIONum;
    gpio.config = DUET_OUTPUT_PUSH_PULL;
    duet_gpio_init(&gpio);
}

void LEDManager::Invert(void)
{
    Set(!mState);
}

void LEDManager::Set(bool state)
{
    mBlinkOnTimeMS  = 0;
    mBlinkOffTimeMS = 0;
    DoSet(state);
}

void LEDManager::Blink(uint32_t changeRateMS)
{
    Blink(changeRateMS, changeRateMS);
}

void LEDManager::Blink(uint32_t onTimeMS, uint32_t offTimeMS)
{
    mBlinkOnTimeMS  = onTimeMS;
    mBlinkOffTimeMS = offTimeMS;
    Animate();
}

void LEDManager::Animate()
{
    if (mBlinkOnTimeMS != 0 && mBlinkOffTimeMS != 0)
    {
        uint64_t nowMS            = chip::System::SystemClock().GetMonotonicMilliseconds64().count();
        uint64_t stateDurMS       = ((mState) ? mBlinkOnTimeMS : mBlinkOffTimeMS);
        uint64_t nextChangeTimeMS = mLastChangeTimeMS + stateDurMS;

        if (nextChangeTimeMS < nowMS)
        {
            DoSet(!mState);
            mLastChangeTimeMS = nowMS;
        }
    }
}

void LEDManager::DoSet(bool state)
{
    mState = state;

    if (state)
    {
        duet_gpio_output_low(&gpio);
    }
    else
    {
        duet_gpio_output_high(&gpio);
    }
}
