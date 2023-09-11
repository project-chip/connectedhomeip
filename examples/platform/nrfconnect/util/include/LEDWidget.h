/*
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>

#include <zephyr/kernel.h>

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
