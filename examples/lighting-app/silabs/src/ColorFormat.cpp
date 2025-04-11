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

// to the specified range given as min and max
RgbColor_t ColorConverter::RgbClamp(uint8_t r, uint8_t g, uint8_t b, uint8_t min, uint8_t max)
{
    RgbColor_t rgb;
    rgb.r = (uint8_t) clamp(r, min, max);
    rgb.g = (uint8_t) clamp(g, min, max);
    rgb.b = (uint8_t) clamp(b, min, max);
    return rgb;
}

// Calculate the red component based on color temperature in centiKelvin
float ColorConverter::CalculateRed(float ctCentiKelvin)
{
    if (ctCentiKelvin <= 66)
    {
        return 255;
    }
    else
    {
        return 329.698727446f * pow(ctCentiKelvin - 60, -0.1332047592f);
    }
}

// Calculate the green component based on color temperature in centiKelvin
float ColorConverter::CalculateGreen(float ctCentiKelvin)
{
    if (ctCentiKelvin <= 66)
    {
        return 99.4708025861f * log(ctCentiKelvin) - 161.1195681661f;
    }
    else
    {
        return 288.1221695283f * pow(ctCentiKelvin - 60, -0.0755148492f);
    }
}

// Calculate the blue component based on color temperature in centiKelvin
float ColorConverter::CalculateBlue(float ctCentiKelvin)
{
    if (ctCentiKelvin >= 66)
    {
        return 255;
    }
    else
    {
        if (ctCentiKelvin <= 19)
        {
            return 0;
        }
        else
        {
            return 138.5177312231f * log(ctCentiKelvin - 10) - 305.0447927307f;
        }
    }
}

// Apply gamma correction to the given value
float ColorConverter::ApplyGammaCorrection(float value)
{
    return (value <= 0.00304f) ? 12.92f * value : (1.055f * std::pow(value, 1.0f / 2.4f)) - 0.055f;
}

RgbColor_t ColorConverter::NormalizeRgb(float r, float g, float b)
{
    RgbColor_t rgb;
    rgb.r = static_cast<uint8_t>(r * 255);
    rgb.g = static_cast<uint8_t>(g * 255);
    rgb.b = static_cast<uint8_t>(b * 255);
    return rgb;
}

// Convert XYZ color space to RGB color space
RgbColor_t ColorConverter::ConvertXYZToRGB(float X, float Y, float Z)
{
    // X, Y and Z input refer to a D65/2° standard illuminant.
    // sR, sG and sB (standard RGB) output range = 0 ÷ 255
    // convert XYZ to RGB - CIE XYZ to sRGB
    RgbColor_t srgb;
    float r = (X * 3.2410f) - (Y * 1.5374f) - (Z * 0.4986f);
    float g = -(X * 0.9692f) + (Y * 1.8760f) + (Z * 0.0416f);
    float b = (X * 0.0556f) - (Y * 0.2040f) + (Z * 1.0570f);

    // Apply gamma 2.2 correction
    r = ApplyGammaCorrection(r);
    g = ApplyGammaCorrection(g);
    b = ApplyGammaCorrection(b);

    // Clamp the gamma-corrected RGB values to the range 0.0 to 1.0
    r = clamp(r, 0.0f, 1.0f);
    g = clamp(g, 0.0f, 1.0f);
    b = clamp(b, 0.0f, 1.0f);

    // Convert the clamped RGB values to the 0-255 range
    RgbColor_t rgb = NormalizeRgb(r, g, b);

    return rgb;
}

// Calculate the P value for HSV to RGB conversion
uint8_t ColorConverter::CalculateP(uint8_t v, uint8_t s)
{
    // RGB value when the hue is at its minimum for the current region.
    return (v * (255 - s)) >> 8;
}

// Calculate the Q value for HSV to RGB conversion
uint8_t ColorConverter::CalculateQ(uint8_t v, uint8_t s, uint32_t remainder)
{
    // RGB value when the hue is transitioning from one primary color to another within the current region.
    return (v * (255 - ((s * remainder) >> 8))) >> 8;
}

// Calculate the T value for HSV to RGB conversion
uint8_t ColorConverter::CalculateT(uint8_t v, uint8_t s, uint32_t remainder)
{
    // RGB value when the hue is transitioning from one primary color to another within the current region, but in the opposite
    // direction of q.

    return (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;
}

// Set RGB values based on the region for HSV to RGB conversion
void ColorConverter::SetRgbByRegion(uint8_t region, uint8_t v, uint8_t p, uint8_t q, uint8_t t, RgbColor_t & rgb)
{
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
    default:
        rgb.r = v, rgb.g = p, rgb.b = q;
        break;
    }
}

// Convert color temperature to RGB color space
RgbColor_t ColorConverter::CTToRgb(CtColor_t ct)
{
    RgbColor_t rgb;
    float ctCentiKelvin = 10000 / static_cast<float>(ct.ctMireds);

    float r = CalculateRed(ctCentiKelvin);
    float g = CalculateGreen(ctCentiKelvin);
    float b = CalculateBlue(ctCentiKelvin);

    rgb = RgbClamp(r, g, b, 0, 255);

    return rgb;
}

// Convert HSV color space to RGB color space
RgbColor_t ColorConverter::HsvToRgb(HsvColor_t hsv)
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

        // region variable is calculated by dividing the hue value by 43 (since the hue value ranges from 0 to 255, and there are 6
        // regions, each region spans approximately 43 units)
        region = h / 43;

        // Calculates the position of h within the current region.Multiplying by 6 scales this position to a range of 0 to 255,
        // which is used to calculate intermediate RGB values.
        remainder = (h - (region * 43)) * 6;

        // p,q and t intermediate values used to calculate the final RGB values
        p = CalculateP(v, s);
        q = CalculateQ(v, s, remainder);
        t = CalculateT(v, s, remainder);

        SetRgbByRegion(region, v, p, q, t, rgb);
    }

    return rgb;
}

// Convert XY color space to RGB color space
RgbColor_t ColorConverter::XYToRgb(uint8_t Level, uint16_t currentX, uint16_t currentY)
{
    // Converts the 16 - bit currentY value to a normalized y value in the range 0.0 to 1.0
    float x = static_cast<float>(currentX) / 65535.0f;
    float y = static_cast<float>(currentY) / 65535.0f;

    // Calculates the z value as 1.0 - x - y, ensuring the sum of x, y, and z equals 1.0.
    float z = 1.0f - x - y;

    // Converts the brightness level Level to a normalized Y value in the range 0.0 to 1.0.
    float Y = static_cast<float>(Level) / 254.0f;
    float X = (Y / y) * x;
    float Z = (Y / y) * z;

    return ConvertXYZToRGB(X, Y, Z);
}
