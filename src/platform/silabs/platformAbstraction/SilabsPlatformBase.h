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
    virtual uint32_t GetRebootCause() = 0;

    // Scheduler
    virtual void StartScheduler(void) = 0;

    // Buttons
    typedef void (*SilabsButtonCb)(uint8_t, uint8_t);
    virtual void SetButtonsCb(SilabsButtonCb callback) {}
    virtual uint8_t GetButtonState(uint8_t button) { return 0; }

    // LEDS
    virtual void InitLed(void) {}
    virtual CHIP_ERROR SetLed(bool state, uint8_t led) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    virtual bool GetLedState(uint8_t led) { return 0; }
    virtual CHIP_ERROR ToggleLed(uint8_t led) { return CHIP_ERROR_NOT_IMPLEMENTED; }

    // Flash
    virtual CHIP_ERROR FlashInit() { return CHIP_ERROR_NOT_IMPLEMENTED; }
    virtual CHIP_ERROR FlashErasePage(uint32_t addr) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    virtual CHIP_ERROR FlashWritePage(uint32_t addr, const uint8_t * data, size_t size) { return CHIP_ERROR_NOT_IMPLEMENTED; }

    /**
     * @brief Function trigger the platform to execute a software reset.
     *              Anything after this function will not be executed since the device will reboot.
     */
    virtual void SoftwareReset(void) = 0;

    // BLE Specific Method

protected:
    SilabsPlatformAbstractionBase(){};
    ~SilabsPlatformAbstractionBase(){};
};

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
