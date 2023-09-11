/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

class LEDWidget
{
public:
    typedef void (*LEDWidgetStateUpdateHandler)(LEDWidget * ledWidget);

    static void SetStateUpdateCallback(LEDWidgetStateUpdateHandler stateUpdateCb);
    void Init(gpio_dt_spec gpio);
    void Set(bool state);
    void Invert(void);
    void Blink(uint32_t changeRateMS);
    void Blink(uint32_t onTimeMS, uint32_t offTimeMS);
    void UpdateState();

private:
    uint32_t mBlinkOnTimeMS;
    uint32_t mBlinkOffTimeMS;
    gpio_dt_spec mGPIO;
    bool mState;
    k_timer mLedTimer;

    static void LedStateTimerHandler(k_timer * timer);

    void DoSet(bool state);
    void ScheduleStateChange();
};
