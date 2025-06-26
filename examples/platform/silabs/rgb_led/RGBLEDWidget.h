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

#include "LEDWidget.h"
#include <stdint.h>

class RGBLEDWidget : public LEDWidget
{
public:
    // Helper Structures
    struct RgbColor_t
    {
        uint8_t r, g, b;
    };

    struct XyColor_t
    {
        uint16_t x, y;
    };

    struct HsvColor_t
    {
        uint8_t h, s, v;
    };

    struct CtColor_t
    {
        uint16_t ctMireds;
    };

    // Union of possible color formats
    union ColorData_t
    {
        HsvColor_t hsv;
        XyColor_t xy;
        CtColor_t ct;
    };

    // Setters and Getters
    void SetColor(uint8_t r, uint8_t g, uint8_t b);
    void GetColor(uint16_t & r, uint16_t & g, uint16_t & b);

    // Color Conversion Functions
    void SetColorFromHSV(uint8_t h, uint8_t s);
    void SetColorFromXY(uint16_t currentX, uint16_t currentY);
    void SetColorFromCT(uint16_t ctMireds);

private:
    // Helper functions for color conversion
    RgbColor_t HsvToRgb(HsvColor_t hsv);
    RgbColor_t XYToRgb(uint8_t level, uint16_t currentX, uint16_t currentY);
    RgbColor_t CTToRgb(uint16_t ct);

    // Color adjustments and conversions
    RgbColor_t RgbClamp(RgbColor_t rgb, uint8_t min, uint8_t max);
    RgbColor_t ConvertXYZToRGB(float X, float Y, float Z);
    RgbColor_t NormalizeRgb(float r, float g, float b);
    float ApplyGammaCorrection(float value);
    float CalculateRed(float ct);
    float CalculateGreen(float ct);
    float CalculateBlue(float ct);

    uint8_t CalculateP(uint8_t v, uint8_t s);
    uint8_t CalculateQ(uint8_t v, uint8_t s, uint32_t remainder);
    uint8_t CalculateT(uint8_t v, uint8_t s, uint32_t remainder);
    void SetRgbByRegion(uint8_t region, uint8_t v, uint8_t p, uint8_t q, uint8_t t, RgbColor_t & rgb);
};
