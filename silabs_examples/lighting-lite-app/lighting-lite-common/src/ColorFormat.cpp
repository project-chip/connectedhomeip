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

#include "ColorFormat.h"

#include <math.h>

// define a clamp macro to substitute the std::clamp macro which is available from C++17 onwards
#define clamp(a, min, max) ((a) < (min) ? (min) : ((a) > (max) ? (max) : (a)))

RgbColor_t HsvToRgb(HsvColor_t hsv)
{
    RgbColor_t rgb;

    uint16_t i       = hsv.h / 60;
    uint16_t rgb_max = hsv.v;
    uint16_t rgb_min = (uint16_t)(rgb_max * (100 - hsv.s)) / 100;
    uint16_t diff    = hsv.h % 60;
    uint16_t rgb_adj = (uint16_t)((rgb_max - rgb_min) * diff) / 60;

    switch (i)
    {
    case 0:
        rgb.r = (uint8_t) rgb_max;
        rgb.g = (uint8_t)(rgb_min + rgb_adj);
        rgb.b = (uint8_t) rgb_min;
        break;
    case 1:
        rgb.r = (uint8_t)(rgb_max - rgb_adj);
        rgb.g = (uint8_t) rgb_max;
        rgb.b = (uint8_t) rgb_min;
        break;
    case 2:
        rgb.r = (uint8_t) rgb_min;
        rgb.g = (uint8_t) rgb_max;
        rgb.b = (uint8_t)(rgb_min + rgb_adj);
        break;
    case 3:
        rgb.r = (uint8_t) rgb_min;
        rgb.g = (uint8_t)(rgb_max - rgb_adj);
        rgb.b = (uint8_t) rgb_max;
        break;
    case 4:
        rgb.r = (uint8_t)(rgb_min + rgb_adj);
        rgb.g = (uint8_t) rgb_min;
        rgb.b = (uint8_t) rgb_max;
        break;
    default:
        rgb.r = (uint8_t) rgb_max;
        rgb.g = (uint8_t) rgb_min;
        rgb.b = (uint8_t)(rgb_max - rgb_adj);
        break;
    }

    return rgb;
}

RgbColor_t XYToRgb(uint8_t Level, uint16_t currentX, uint16_t currentY)
{
    // convert xyY color space to RGB

    // https://www.easyrgb.com/en/math.php
    // https://en.wikipedia.org/wiki/SRGB
    // refer https://en.wikipedia.org/wiki/CIE_1931_color_space#CIE_xy_chromaticity_diagram_and_the_CIE_xyY_color_space

    // The currentX/currentY attribute contains the current value of the normalized chromaticity value of x/y.
    // The value of x/y shall be related to the currentX/currentY attribute by the relationship
    // x = currentX/65536
    // y = currentY/65536
    // z = 1-x-y

    RgbColor_t rgb;

    float x, y, z;
    float X, Y, Z;
    float r, g, b;

    x = ((float) currentX) / 65535.0f;
    y = ((float) currentY) / 65535.0f;

    z = 1.0f - x - y;

    // Calculate XYZ values

    // Y - given brightness in 0 - 1 range
    Y = ((float) Level) / 254.0f;
    X = (Y / y) * x;
    Z = (Y / y) * z;

    // X, Y and Z input refer to a D65/2° standard illuminant.
    // sR, sG and sB (standard RGB) output range = 0 ÷ 255
    // convert XYZ to RGB - CIE XYZ to sRGB
    X = X / 100.0f;
    Y = Y / 100.0f;
    Z = Z / 100.0f;

    r = (X * 3.2406f) - (Y * 1.5372f) - (Z * 0.4986f);
    g = -(X * 0.9689f) + (Y * 1.8758f) + (Z * 0.0415f);
    b = (X * 0.0557f) - (Y * 0.2040f) + (Z * 1.0570f);

    // apply gamma 2.2 correction
    r = (r <= 0.0031308f ? 12.92f * r : (1.055f) * pow(r, (1.0f / 2.4f)) - 0.055f);
    g = (g <= 0.0031308f ? 12.92f * g : (1.055f) * pow(g, (1.0f / 2.4f)) - 0.055f);
    b = (b <= 0.0031308f ? 12.92f * b : (1.055f) * pow(b, (1.0f / 2.4f)) - 0.055f);

    // Round off
    r = clamp(r, 0, 1);
    g = clamp(g, 0, 1);
    b = clamp(b, 0, 1);

    // these rgb values are in  the range of 0 to 1, convert to limit of HW specific LED
    rgb.r = (uint8_t)(r * 255);
    rgb.g = (uint8_t)(g * 255);
    rgb.b = (uint8_t)(b * 255);

    return rgb;
}
