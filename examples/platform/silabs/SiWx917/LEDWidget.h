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

#pragma once

#include <stdint.h>

extern "C" void RSI_Board_LED_Set(int, int);
extern "C" void RSI_Board_LED_Toggle(int);

class LEDWidget
{
public:
    void Init(int led);
    void Set(bool state);
    void Invert(void);
    void Blink(uint32_t changeRateMS);
    void Blink(uint32_t onTimeMS, uint32_t offTimeMS);
    void Animate();

private:
    uint64_t mLastChangeTimeMS;
    uint32_t mBlinkOnTimeMS;
    uint32_t mBlinkOffTimeMS;
    int mLed;
    // created a temporary mLedStatus since Led status is not updating from the platform API(RSI_EGPIO_GetPin)
    bool mLedStatus;
};
