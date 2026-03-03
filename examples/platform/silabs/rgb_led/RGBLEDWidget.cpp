/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "RGBLEDWidget.h"
#include <algorithm>
#include <math.h>

#include <platform/CHIPDeviceLayer.h>
#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

/*Delay for PWM simulation*/
#ifndef TICK_DELAY
#define TICK_DELAY 2
#endif

using namespace ::chip::System;
using namespace chip::DeviceLayer::Silabs;

using RgbColor_t = RGBLEDWidget::RgbColor_t;
using HsvColor_t = RGBLEDWidget::HsvColor_t;

void RGBLEDWidget::SetColor(uint8_t red, uint8_t green, uint8_t blue)
{
    if (GetPlatform().GetRGBLedState(GetLED()))
    {
        ChipLogDetail(Zcl, "SetColor : R:%u|G:%u|B:%u", red, green, blue);
        TEMPORARY_RETURN_IGNORED GetPlatform().SetLedColor(GetLED(), red, green, blue);
    }
}

void RGBLEDWidget::GetColor(uint16_t & r, uint16_t & g, uint16_t & b)
{
    uint16_t rawR, rawG, rawB;
    TEMPORARY_RETURN_IGNORED GetPlatform().GetLedColor(GetLED(), rawR, rawG, rawB);

    // Scale down PWM values by TICK_DELAY to bring them back to the expected range
    // Clamp to 8-bit range (0-255)
    r = static_cast<uint16_t>((rawR / TICK_DELAY) & 0xFF);
    g = static_cast<uint16_t>((rawG / TICK_DELAY) & 0xFF);
    b = static_cast<uint16_t>((rawB / TICK_DELAY) & 0xFF);

    ChipLogDetail(Zcl, "RGB Values: R=%u G=%u B=%u", r, g, b);
}
/**
 * @brief Convert Data model hue and saturation to RGB and set the color
 *        As per spec, hue range from 0 to 254 and saturation from 0 to 254
 *        Since the equation expects hue in 0-360 and saturation in percentage, we scale them accordingly
 *
 * @param hue
 * @param saturation
 */
void RGBLEDWidget::SetColorFromHSV(uint8_t hue, uint8_t saturation)
{
    ChipLogDetail(Zcl, "SetColorFromHSV : H:%u|S:%u|V:%u", hue, saturation, GetLevel());
    // Per spec max value for each attribute is 254
    float h = (hue * 360.0f) / 254.0f, s = (saturation / 254.0f), v = (GetLevel() / 254.0f);
    float hh, p, q, t, ff, r, g, b;
    long i;

    ChipLogDetail(Zcl, "HUE Values: H=%u S=%u V=%u", static_cast<uint8_t>(h), static_cast<uint8_t>(s * 100.0),
                  static_cast<uint8_t>(v * 100.0));

    if (s <= 0.0)
    { // Achromatic (grey)
        SetColor(static_cast<uint8_t>(v * 255), static_cast<uint8_t>(v * 255), static_cast<uint8_t>(v * 255));
        return;
    }

    hh = h;
    if (hh >= 360.0)
        hh = 0.0;
    hh /= 60.0;
    i  = (long) hh;
    ff = hh - i;
    p  = v * (1.0 - s);
    q  = v * (1.0 - (s * ff));
    t  = v * (1.0 - (s * (1.0 - ff)));

    switch (i)
    {
    case 0:
        r = v;
        g = t;
        b = p;
        break;
    case 1:
        r = q;
        g = v;
        b = p;
        break;
    case 2:
        r = p;
        g = v;
        b = t;
        break;
    case 3:
        r = p;
        g = q;
        b = v;
        break;
    case 4:
        r = t;
        g = p;
        b = v;
        break;
    case 5:
    default:
        r = v;
        g = p;
        b = q;
        break;
    }

    SetColor(static_cast<uint8_t>(r * 255), static_cast<uint8_t>(g * 255), static_cast<uint8_t>(b * 255));
}

void RGBLEDWidget::SetColorFromXY(uint16_t currentX, uint16_t currentY)
{
    ChipLogDetail(Zcl, "SetColorFromXY: %u|%u", currentX, currentY);
    RGBLEDWidget::RgbColor_t rgb = RGBLEDWidget::XYToRgb(GetLevel(), currentX, currentY);
    SetColor(rgb.r, rgb.g, rgb.b);
}

void RGBLEDWidget::SetColorFromCT(uint16_t ctMireds)
{
    RGBLEDWidget::RgbColor_t rgb = RGBLEDWidget::CTToRgb(ctMireds);
    ChipLogDetail(Zcl, "SetColorFromCT: %u", ctMireds);
    SetColor(rgb.r, rgb.g, rgb.b);
}

// Constant values
constexpr float kMaxChromaticity    = 65535.0f;
constexpr float kMaxBrightnessLevel = 254.0f;
constexpr float kGammaThreshold     = 0.00304f;
constexpr float kGammaMultiplier    = 12.92f;
constexpr float kGammaExponent      = 1.0f / 2.4f;
constexpr float kGammaOffset        = 0.055f;
constexpr float kHueRegionDivider   = 43.0f;
constexpr float kHueScaleFactor     = 6.0f;
constexpr uint8_t kMaxColorValue    = 255;

// to the specified range given as min and max
RgbColor_t RGBLEDWidget::RgbClamp(RgbColor_t rgb, uint8_t min, uint8_t max)
{
    rgb.r = static_cast<uint8_t>(std::clamp(rgb.r, min, max));
    rgb.g = static_cast<uint8_t>(std::clamp(rgb.g, min, max));
    rgb.b = static_cast<uint8_t>(std::clamp(rgb.b, min, max));
    return rgb;
}

// Calculate the red component based on color temperature in centiKelvin
float RGBLEDWidget::CalculateRed(float ctCentiKelvin)
{
    constexpr float kRedThreshold  = 66.0f;
    constexpr float kRedMultiplier = 329.698727446f;
    constexpr float kRedExponent   = -0.1332047592f;
    if (ctCentiKelvin <= kRedThreshold)
    {
        return kMaxColorValue;
    }
    else
    {
        return kRedMultiplier * pow(ctCentiKelvin - 60, kRedExponent);
    }
}

// Calculate the green component based on color temperature in centiKelvin
float RGBLEDWidget::CalculateGreen(float ctCentiKelvin)
{
    constexpr float kGreenThreshold      = 66.0f;
    constexpr float kGreenMultiplierLow  = 99.4708025861f;
    constexpr float kGreenOffsetLow      = -161.1195681661f;
    constexpr float kGreenMultiplierHigh = 288.1221695283f;
    constexpr float kGreenExponentHigh   = -0.0755148492f;
    if (ctCentiKelvin <= kGreenThreshold)
    {
        return kGreenMultiplierLow * log(ctCentiKelvin) - kGreenOffsetLow;
    }
    else
    {
        return kGreenMultiplierHigh * pow(ctCentiKelvin - 60, kGreenExponentHigh);
    }
}

// Calculate the blue component based on color temperature in centiKelvin
float RGBLEDWidget::CalculateBlue(float ctCentiKelvin)
{
    constexpr float kBlueThresholdHigh = 66.0f;
    constexpr float kBlueThresholdLow  = 19.0f;
    constexpr float kBlueMultiplier    = 138.5177312231f;
    constexpr float kBlueOffset        = -305.0447927307f;
    if (ctCentiKelvin >= kBlueThresholdHigh)
    {
        return kMaxColorValue;
    }
    else
    {
        if (ctCentiKelvin <= kBlueThresholdLow)
        {
            return 0;
        }
        else
        {
            return kBlueMultiplier * log(ctCentiKelvin - 10) - kBlueOffset;
        }
    }
}

// Apply gamma correction to the given value
float RGBLEDWidget::ApplyGammaCorrection(float value)
{
    // If the values are below a certain threshold (0.00304), the correction is simple (12.92 * value), otherwise, the gamma
    // curve is applied This step ensures that the colors look good when displayed on typical RGB devices
    return (value <= kGammaThreshold) ? kGammaMultiplier * value : (1.055f * std::pow(value, kGammaExponent)) - kGammaOffset;
}

RgbColor_t RGBLEDWidget::NormalizeRgb(float r, float g, float b)
{
    RgbColor_t rgb;
    rgb.r = static_cast<uint8_t>(r * kMaxColorValue);
    rgb.g = static_cast<uint8_t>(g * kMaxColorValue);
    rgb.b = static_cast<uint8_t>(b * kMaxColorValue);
    return rgb;
}

// Convert XYZ color space to RGB color space
RgbColor_t RGBLEDWidget::ConvertXYZToRGB(float X, float Y, float Z)
{
    // X, Y and Z input refer to a D65/2° standard illuminant.
    // The XYZ values are then converted to RGB values using a standard transformation matrix for the sRGB color space (D65
    // illuminant, 2° observer):
    // These coefficients come from a linear transformation from XYZ to sRGB color space, which is based on how human vision
    // perceives color.
    constexpr float kMatrixR1 = 3.2410f;
    constexpr float kMatrixR2 = -1.5374f;
    constexpr float kMatrixR3 = -0.4986f;
    constexpr float kMatrixG1 = -0.9692f;
    constexpr float kMatrixG2 = 1.8760f;
    constexpr float kMatrixG3 = 0.0416f;
    constexpr float kMatrixB1 = 0.0556f;
    constexpr float kMatrixB2 = -0.2040f;
    constexpr float kMatrixB3 = 1.0570f;

    // Convert XYZ to RGB using the transformation matrix
    float r = (X * kMatrixR1) + (Y * kMatrixR2) + (Z * kMatrixR3);
    float g = (X * kMatrixG1) + (Y * kMatrixG2) + (Z * kMatrixG3);
    float b = (X * kMatrixB1) + (Y * kMatrixB2) + (Z * kMatrixB3);

    // Apply gamma 2.2 correction
    r = ApplyGammaCorrection(r);
    g = ApplyGammaCorrection(g);
    b = ApplyGammaCorrection(b);

    // Clamp the gamma-corrected RGB values to the range 0.0 to 1.0
    r = std::clamp(r, 0.0f, 1.0f);
    g = std::clamp(g, 0.0f, 1.0f);
    b = std::clamp(b, 0.0f, 1.0f);

    // Convert the clamped RGB values to the 0-255 range
    RgbColor_t rgb = NormalizeRgb(r, g, b);

    return rgb;
}

// Convert color temperature to RGB color space
RgbColor_t RGBLEDWidget::CTToRgb(uint16_t ctMireds)
{
    RgbColor_t rgb;
    float ctCentiKelvin = 10000 / static_cast<float>(ctMireds);

    rgb.r = CalculateRed(ctCentiKelvin);
    rgb.g = CalculateGreen(ctCentiKelvin);
    rgb.b = CalculateBlue(ctCentiKelvin);

    rgb = RgbClamp(rgb, 0, kMaxColorValue);

    return rgb;
}

// Convert XY color space to RGB color space
RgbColor_t RGBLEDWidget::XYToRgb(uint8_t Level, uint16_t currentX, uint16_t currentY)
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

    // Converts the 16 - bit currentX,currentY value to a normalized y value in the range 0.0 to 1.0
    constexpr float kMaxChromaticity    = 65535.0f;
    constexpr float kMaxBrightnessLevel = 254.0f;

    float x = static_cast<float>(currentX) / kMaxChromaticity;
    float y = static_cast<float>(currentY) / kMaxChromaticity;

    // Calculates the z value as 1.0 - x - y, ensuring the sum of x, y, and z equals 1.0.
    // Needed to convert the xy values to full XYZ format
    float z = 1.0f - x - y;

    // Converts the brightness level Level to a normalized Y value in the range 0.0 to 1.0.
    float Y = static_cast<float>(Level) / kMaxBrightnessLevel;

    // chromaticity (x and y) to get the full XYZ color space
    float X = (Y / y) * x;
    float Z = (Y / y) * z;

    return ConvertXYZToRGB(X, Y, Z);
}
