/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#pragma once

#include "matter_led.h"
#include <app/util/basic-types.h>
#include <cstdio>

class LEDWidget
{
public:
    LEDWidget() : mLEDHandle(NULL) {}

    static void InitGpio();
    void Init(uint8_t gpioNum);
    void Set(bool state);
    void Invert(void);
    void Blink(uint32_t changeRateMS);
    void Blink(uint32_t onTimeMS, uint32_t offTimeMS);
    void BlinkStop();
    bool IsTurnedOn() { return mState; }

    void Init(uint8_t gpioNum, uint8_t aMinLevel, uint8_t aMaxLevel, uint8_t aDefaultLevel);
    uint8_t GetLevel() const { return mLevel; }
    uint8_t GetMinLevel() const { return mMinLevel; }
    uint8_t GetMaxLevel() const { return mMaxLevel; }
    void SetLevel(uint8_t aLevel);

private:
    T_MATTER_LED_HANDLE mLEDHandle;
    bool mState;
    uint8_t mMinLevel;
    uint8_t mMaxLevel;
    uint8_t mLevel;
};
