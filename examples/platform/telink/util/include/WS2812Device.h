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

#include <ColorFormat.h>

#include <zephyr/drivers/led_strip.h>

#include <system/SystemError.h>

#define STRIP_NUM_PIXELS(LED_STRIP) DT_PROP(DT_ALIAS(LED_STRIP), chain_length)
#define RGB_MIN_VALUE 0
#define RGB_MAX_VALUE 255

#define SET_RGB_TURN_OFF 0
#define SET_RGB_TURN_ON 1

class WS2812Device
{
public:
    CHIP_ERROR Init(const struct device * ws2812Device, uint32_t aChainLength);
    void SetLevel(RgbColor_t * pRgb);
    void Set(bool aTurnOn);
    bool IsTurnedOn(void);
    uint8_t GetBlueLevel(void) const { return mLedRgb.b; }
    uint8_t GetGreenLevel(void) const { return mLedRgb.g; }
    uint8_t GetRedLevel(void) const { return mLedRgb.r; }

private:
    enum WS2812State_t : uint8_t
    {
        kRgbState_On = 0,
        kRgbState_Off,
    };

    void UpdateRgbLight();

    const struct device * mWs2812Device;
    uint32_t mChainLength;
    RgbColor_t mLedRgb;
    WS2812State_t mState;
};
