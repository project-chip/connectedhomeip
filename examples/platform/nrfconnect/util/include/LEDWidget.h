/*
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

#include <cstdint>

#include <zephyr/zephyr.h>

class LEDWidget
{
public:
    typedef void (*LEDWidgetStateUpdateHandler)(LEDWidget & ledWidget);

    static void InitGpio();
    static void SetStateUpdateCallback(LEDWidgetStateUpdateHandler stateUpdateCb);
    void Init(uint32_t gpioNum);
    void Set(bool state);
    void Invert(void);
    void Blink(uint32_t changeRateMS);
    void Blink(uint32_t onTimeMS, uint32_t offTimeMS);
    void UpdateState();

private:
    uint32_t mBlinkOnTimeMS;
    uint32_t mBlinkOffTimeMS;
    uint32_t mGPIONum;
    bool mState;
    k_timer mLedTimer;

    static void LedStateTimerHandler(k_timer * timer);

    void DoSet(bool state);
    void ScheduleStateChange();
};
