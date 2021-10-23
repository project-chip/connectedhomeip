/*
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

#pragma once

#include <cstdint>
#include <drivers/gpio.h>

class LEDWidget
{
public:
    static void InitGpio(const device * port);
    const static struct device * mPort;
    void Init(gpio_pin_t gpioNum);
    void Set(bool state);
    void Invert(void);
    void Blink(uint32_t changeRateMS);
    void Blink(uint32_t onTimeMS, uint32_t offTimeMS);
    void Animate();

private:
    int64_t mLastChangeTimeMS;
    uint32_t mBlinkOnTimeMS;
    uint32_t mBlinkOffTimeMS;
    gpio_pin_t mGPIONum;
    bool mState;

    void DoSet(bool state);
};
