/*
 *
 * SPDX-FileCopyrightText: 2020 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "LED.h"

#pragma once

class LEDWidget
{
public:
    void Init(LED_t gpioNum);
    void Set(bool state);
    void Invert(void);
    void Blink(uint32_t changeRateMS);
    void Blink(uint32_t onTimeMS, uint32_t offTimeMS);
    void Animate();

private:
    uint64_t mLastChangeTimeMS;
    uint32_t mBlinkOnTimeMS;
    uint32_t mBlinkOffTimeMS;
    LED_t mGPIONum;
    bool mState;

    void DoSet(bool state);
};
