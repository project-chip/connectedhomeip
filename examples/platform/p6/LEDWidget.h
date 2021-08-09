/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "cybsp.h"
#include <stdint.h>

class LEDWidget
{
public:
    static void InitGpio(void);
    void Init(int ledNum);
    void Set(bool state);
    void Invert(void);
    void Blink(uint32_t changeRateMS);
    void Blink(uint32_t onTimeMS, uint32_t offTimeMS);
    void Animate();

private:
    int64_t mLastChangeTimeUS = 0;
    uint32_t mBlinkOnTimeMS   = 0;
    uint32_t mBlinkOffTimeMS  = 0;
    int mLedNum               = 0;
    bool mState               = CYBSP_LED_STATE_OFF;

    void DoSet(bool state);
};
