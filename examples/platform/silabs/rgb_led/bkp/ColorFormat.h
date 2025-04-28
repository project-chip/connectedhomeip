/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
class ColorConverter
{
public:
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
    static RgbColor_t HsvToRgb(HsvColor_t hsv);
    static RgbColor_t XYToRgb(uint8_t Level, uint16_t currentX, uint16_t currentY);
    static RgbColor_t CTToRgb(CtColor_t ct);

private:
    static RgbColor_t RgbClamp(uint8_t r, uint8_t g, uint8_t b, uint8_t min, uint8_t max);
    static RgbColor_t ConvertXYZToRGB(float X, float Y, float Z);
    static RgbColor_t NormalizeRgb(float r, float g, float b);
    static float ApplyGammaCorrection(float value);
    static float CalculateRed(float ctCentiKelvin);
    static float CalculateGreen(float ctCentiKelvin);
    static float CalculateBlue(float ctCentiKelvin);

    static uint8_t CalculateP(uint8_t v, uint8_t s);
    static uint8_t CalculateQ(uint8_t v, uint8_t s, uint32_t remainder);
    static uint8_t CalculateT(uint8_t v, uint8_t s, uint32_t remainder);

    static void SetRgbByRegion(uint8_t region, uint8_t v, uint8_t p, uint8_t q, uint8_t t, RgbColor_t & rgb);
};
