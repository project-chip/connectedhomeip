/*
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <stdio.h>

#include <lib/core/CHIPError.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {

class SilabsPlatformAbstractionBase
{

public:
    // Generic Peripherical methods
    virtual CHIP_ERROR Init(void) = 0;
    virtual CHIP_ERROR InitLCD() { return CHIP_ERROR_NOT_IMPLEMENTED; }
    virtual CHIP_ERROR SetGPIO(uint32_t port, uint32_t pin, bool state) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    virtual bool GetGPIO(uint32_t port, uint32_t pin) { return false; }

    // Buttons

    // LEDS
    virtual void InitLed(void) {}
    virtual CHIP_ERROR SetLed(bool state, uint8_t led) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    virtual bool GetLedState(uint8_t led) { return 0; }
    virtual CHIP_ERROR ToggleLed(uint8_t led) { return CHIP_ERROR_NOT_IMPLEMENTED; }

    // BLE Specific Method

protected:
    SilabsPlatformAbstractionBase(){};
    ~SilabsPlatformAbstractionBase(){};
};

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
