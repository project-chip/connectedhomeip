/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <lib/core/CHIPError.h>
#include <wiced_led_manager.h>

class LightingManager
{
public:
    wiced_result_t Init();
    void Set(bool state, uint8_t pin);
    void Blink(wiced_led_t led_pin, uint32_t on_period_ms, uint32_t off_period_ms);
    bool IsLightOn();

private:
    void DoSetLEDOnOff(bool state, uint8_t pin);

    friend LightingManager & LightMgr(void);

    bool usr_LED1_OnOffStatus;

    static LightingManager sLight;
};

inline LightingManager & LightMgr(void)
{
    return LightingManager::sLight;
}
