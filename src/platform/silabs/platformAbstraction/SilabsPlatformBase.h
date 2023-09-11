/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

    // Scheduler
    virtual void StartScheduler(void) = 0;

    // Buttons
    typedef void (*SilabsButtonCb)(uint8_t, uint8_t);
    virtual void SetButtonsCb(SilabsButtonCb callback) {}

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
