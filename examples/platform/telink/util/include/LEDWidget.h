/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <zephyr/drivers/gpio.h>

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
