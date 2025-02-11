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

    uint8_t region, p, q, t;
    uint32_t h, s, v, remainder;

    if (hsv.s == 0)
    {
        rgb.r = rgb.g = rgb.b = hsv.v;
    }
    else
    {
        h = hsv.h;
        s = hsv.s;
        v = hsv.v;

        region    = h / 43;
        remainder = (h - (region * 43)) * 6;
        p         = (v * (255 - s)) >> 8;
        q         = (v * (255 - ((s * remainder) >> 8))) >> 8;
        t         = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;
        switch (region)
        {
        case 0:
            rgb.r = v, rgb.g = t, rgb.b = p;
            break;
        case 1:
            rgb.r = q, rgb.g = v, rgb.b = p;
            break;
        case 2:
            rgb.r = p, rgb.g = v, rgb.b = t;
            break;
        case 3:
            rgb.r = p, rgb.g = q, rgb.b = v;
            break;
        case 4:
            rgb.r = t, rgb.g = p, rgb.b = v;
            break;
        case 5:
        default:
            rgb.r = v, rgb.g = p, rgb.b = q;
            break;
        }
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
    r = (r <= 0.0031308f ? 12.92f * r : (1.055f) * powf(r, (1.0f / 2.4f)) - 0.055f);
    g = (g <= 0.0031308f ? 12.92f * g : (1.055f) * powf(g, (1.0f / 2.4f)) - 0.055f);
    b = (b <= 0.0031308f ? 12.92f * b : (1.055f) * powf(b, (1.0f / 2.4f)) - 0.055f);

    // Round off
    r = clamp(r, 0, 1);
    g = clamp(g, 0, 1);
    b = clamp(b, 0, 1);

    // these rgb values are in  the range of 0 to 1, convert to limit of HW specific LED
    rgb.r = (uint8_t) (r * 255);
    rgb.g = (uint8_t) (g * 255);
    rgb.b = (uint8_t) (b * 255);

    return rgb;
}

RgbColor_t CTToRgb(CtColor_t ct)
{
    RgbColor_t rgb;
    float r, g, b;

    // Algorithm credits to Tanner Helland: https://tannerhelland.com/2012/09/18/convert-temperature-rgb-algorithm-code.html

    // Convert Mireds to centiKelvins. k = 1,000,000/mired
    float ctCentiKelvin = 10000 / ct.ctMireds;

    // Red
    if (ctCentiKelvin <= 66)
    {
        r = 255;
    }
    else
    {
        r = 329.698727446f * powf(ctCentiKelvin - 60, -0.1332047592f);
    }

    // Green
    if (ctCentiKelvin <= 66)
    {
        g = 99.4708025861f * logf(ctCentiKelvin) - 161.1195681661f;
    }
    else
    {
        g = 288.1221695283f * powf(ctCentiKelvin - 60, -0.0755148492f);
    }

    // Blue
    if (ctCentiKelvin >= 66)
    {
        b = 255;
    }
    else
    {
        if (ctCentiKelvin <= 19)
        {
            b = 0;
        }
        else
        {
            b = 138.5177312231 * log(ctCentiKelvin - 10) - 305.0447927307;
        }
    }
    rgb.r = (uint8_t) clamp(r, 0, 255);
    rgb.g = (uint8_t) clamp(g, 0, 255);
    rgb.b = (uint8_t) clamp(b, 0, 255);

    return rgb;
}
