/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
#include "ColorFormat.h"
#endif // (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
#include <stdint.h>

class LEDWidget
{
public:
    static void InitGpio(void);
    void Init(uint8_t led);
    void Set(bool state);
    void Invert(void);
    void Blink(uint32_t changeRateMS);
    void Blink(uint32_t onTimeMS, uint32_t offTimeMS);
    void Animate();
    uint8_t GetLED();
    bool GetLEDStatus(uint8_t led);

private:
    uint64_t mLastChangeTimeMS;
    uint32_t mBlinkOnTimeMS;
    uint32_t mBlinkOffTimeMS;
    uint8_t mLed;
    bool mLedStatus;

protected:
    uint8_t mLevel;
};

#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
class RGBLEDWidget : public LEDWidget
{
public:
    void SetColor(uint8_t red, uint8_t green, uint8_t blue);
    void GetColor(uint16_t red, uint16_t green, uint16_t blue);
    void SetLevel(uint8_t aLevel);
    uint8_t GetLevel();
    void SetColorFromHSV(uint8_t h, uint8_t s);
    void SetColorFromXY(uint16_t currentX, uint16_t currentY);
    void SetColorFromCT(CtColor_t ct);

private:
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};
#endif // (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
