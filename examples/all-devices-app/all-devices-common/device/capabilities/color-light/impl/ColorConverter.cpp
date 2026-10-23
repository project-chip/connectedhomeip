/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <device/capabilities/color-light/impl/ColorConverter.h>

#include <algorithm>
#include <cmath>

namespace chip {
namespace app {
namespace {

// §3.2.7.9: CurrentX/CurrentY are the chromaticity coordinate scaled by 65536, constrained to
// 0xFEFF.
constexpr float kXYScale       = 65536.0f;
constexpr uint16_t kMaxXYValue = 0xFEFF;

// Every correlated color temperature is forced into this window before use, which is what bounds the
// mireds returned here to [153, 370]. Two separate reasons it has to be bounded, and to THIS span:
//
//   * Neither approximation below is defined outside 1667K-25000K, and both diverge sharply past it.
//   * The cluster encodes a delegate's result into ColorTemperatureMireds WITHOUT clamping it, so
//     nothing else keeps the attribute inside its constraint when a read arrives while XY owns the
//     output. The window therefore has to match the colorTempPhysicalMin/MaxMireds the device
//     advertises — see the ctConfig assignments in ColorLight::Register(). Widening one without the
//     other lets an XY-mode read report a temperature the device claims it cannot produce.
constexpr float kMinKelvin = 2700.0f; // 370 mireds, colorTempPhysicalMaxMireds
constexpr float kMaxKelvin = 6535.0f; // 153 mireds, colorTempPhysicalMinMireds

// McCamy's convergence point. The formula is a projection onto the locus through this point, so it
// is undefined on the y == kEpicenterY isotherm and flips sign across it — and that isotherm is a
// perfectly legal CurrentY (~12178 raw), reachable both by MoveToColor and by any MoveColor ramp
// that crosses it.
constexpr float kEpicenterX     = 0.3320f;
constexpr float kEpicenterY     = 0.1858f;
constexpr float kMinDenominator = 1.0e-4f;

constexpr float kKelvinSplit     = 4000.0f;
constexpr float kLowKelvinSplit  = 2222.0f;
constexpr float kMiredsPerKelvin = 1.0e6f;

// §3.2.7.11/.13: CurrentHue and CurrentSaturation are both constrained to 0xFE, hue spanning the full
// 360° circle over that range. 0xFE is the divisor as well as the cap — hue 0xFE is one step short of a
// full revolution, not 360° itself.
constexpr float kMaxHueSatValue = 254.0f;
constexpr float kHueSectors     = 6.0f;

// A chromaticity so far off the locus that no sensible hue exists (y == 0 has no XYZ preimage) falls back
// to the D65-ish white the cluster itself defaults to, rather than to an arbitrary saturated hue.
constexpr float kMinChromaticityY = 1.0e-4f;

// sRGB companding. The conversion runs through sRGB because that is the space the 8-bit hue/saturation
// pair describes; a real product substitutes its own primaries here.
float LinearFromSrgb(float channel)
{
    return (channel <= 0.04045f) ? (channel / 12.92f) : std::pow((channel + 0.055f) / 1.055f, 2.4f);
}

float SrgbFromLinear(float channel)
{
    return (channel <= 0.0031308f) ? (channel * 12.92f) : (1.055f * std::pow(channel, 1.0f / 2.4f) - 0.055f);
}

} // namespace

// McCamy's cubic approximation of the correlated color temperature: the point on the Planckian
// locus closest to the given xy. It only carries meaning for an xy near that locus, and MoveToColor
// accepts any xy in the plane, so the result is constrained rather than trusted.
void ColorConverter::ConvertXYToMireds(uint16_t x, uint16_t y, uint16_t & outMireds)
{
    const float chromaticityX = static_cast<float>(x) / kXYScale;
    const float chromaticityY = static_cast<float>(y) / kXYScale;

    const float denominator = kEpicenterY - chromaticityY;

    // On the isotherm the correlated color temperature diverges; the coolest end of the window is
    // the limit approached from the side the locus lies on.
    float kelvin = kMaxKelvin;
    if (std::fabs(denominator) >= kMinDenominator)
    {
        const float n = (chromaticityX - kEpicenterX) / denominator;
        kelvin        = (((449.0f * n) + 3525.0f) * n + 6823.3f) * n + 5520.33f;
    }

    kelvin    = std::clamp(kelvin, kMinKelvin, kMaxKelvin);
    outMireds = static_cast<uint16_t>(kMiredsPerKelvin / kelvin);
}

// Kim et al.'s cubic-spline fit of the Planckian locus: x as a cubic in 1/T, then y as a cubic in
// x. The two fits split at different temperatures, hence the separate thresholds.
void ColorConverter::ConvertMiredsToXY(uint16_t mireds, uint16_t & outX, uint16_t & outY)
{
    // ColorTemperatureMireds is constrained to colorTempPhysicalMinMireds (>= 1) and up, but the
    // reciprocal is worth guarding independently of that.
    const float rawKelvin = (mireds == 0) ? kMaxKelvin : kMiredsPerKelvin / static_cast<float>(mireds);
    const float kelvin    = std::clamp(rawKelvin, kMinKelvin, kMaxKelvin);

    const float inverseT       = 1.0f / kelvin;
    const float inverseTSquare = inverseT * inverseT;
    const float inverseTCube   = inverseTSquare * inverseT;

    const float chromaticityX = (kelvin <= kKelvinSplit)
        ? (-0.2661239e9f * inverseTCube) - (0.2343589e6f * inverseTSquare) + (0.8776956e3f * inverseT) + 0.179910f
        : (-3.0258469e9f * inverseTCube) + (2.1070379e6f * inverseTSquare) + (0.2226347e3f * inverseT) + 0.240390f;

    float chromaticityY;
    if (kelvin <= kLowKelvinSplit)
    {
        chromaticityY = (((-1.1063814f * chromaticityX) - 1.34811020f) * chromaticityX + 2.18555832f) * chromaticityX - 0.20219683f;
    }
    else if (kelvin <= kKelvinSplit)
    {
        chromaticityY = (((-0.9549476f * chromaticityX) - 1.37418593f) * chromaticityX + 2.09137015f) * chromaticityX - 0.16748867f;
    }
    else
    {
        chromaticityY = (((3.0817580f * chromaticityX) - 5.87338670f) * chromaticityX + 3.75112997f) * chromaticityX - 0.37001483f;
    }

    outX = static_cast<uint16_t>(std::clamp(chromaticityX * kXYScale, 0.0f, static_cast<float>(kMaxXYValue)));
    outY = static_cast<uint16_t>(std::clamp(chromaticityY * kXYScale, 0.0f, static_cast<float>(kMaxXYValue)));
}

// Hue/saturation at full value, through sRGB into CIE xy. Value is pinned at 1.0 because ColorControl
// has no brightness axis of its own — Level Control owns it — so only the chromaticity of the hue/sat
// pair carries over.
void ColorConverter::ConvertHueSatToXY(uint8_t hue, uint8_t sat, uint16_t & outX, uint16_t & outY)
{
    const float saturation = std::clamp(static_cast<float>(sat) / kMaxHueSatValue, 0.0f, 1.0f);
    const float sector     = std::clamp(static_cast<float>(hue) / kMaxHueSatValue, 0.0f, 1.0f) * kHueSectors;

    // Standard HSV sector walk at V == 1: one channel is 1, one is 1 - saturation, and the third ramps
    // between them across the sector.
    const int sextant   = static_cast<int>(sector) % 6;
    const float offset  = sector - std::floor(sector);
    const float lowest  = 1.0f - saturation;
    const float falling = 1.0f - (saturation * offset);
    const float rising  = 1.0f - (saturation * (1.0f - offset));

    float red = 0.0f, green = 0.0f, blue = 0.0f;
    switch (sextant)
    {
    case 0:
        red = 1.0f, green = rising, blue = lowest;
        break;
    case 1:
        red = falling, green = 1.0f, blue = lowest;
        break;
    case 2:
        red = lowest, green = 1.0f, blue = rising;
        break;
    case 3:
        red = lowest, green = falling, blue = 1.0f;
        break;
    case 4:
        red = rising, green = lowest, blue = 1.0f;
        break;
    default:
        red = 1.0f, green = lowest, blue = falling;
        break;
    }

    // sRGB (D65) to CIE XYZ. Companding first: the matrix is defined on linear light.
    const float linearRed   = LinearFromSrgb(red);
    const float linearGreen = LinearFromSrgb(green);
    const float linearBlue  = LinearFromSrgb(blue);

    const float cieX = (0.4124564f * linearRed) + (0.3575761f * linearGreen) + (0.1804375f * linearBlue);
    const float cieY = (0.2126729f * linearRed) + (0.7151522f * linearGreen) + (0.0721750f * linearBlue);
    const float cieZ = (0.0193339f * linearRed) + (0.1191920f * linearGreen) + (0.9503041f * linearBlue);

    const float sum = cieX + cieY + cieZ;
    if (sum <= 0.0f) // only reachable for pure black, which V == 1 excludes; guards the divide regardless
    {
        outX = outY = 0;
        return;
    }

    outX = static_cast<uint16_t>(std::clamp((cieX / sum) * kXYScale, 0.0f, static_cast<float>(kMaxXYValue)));
    outY = static_cast<uint16_t>(std::clamp((cieY / sum) * kXYScale, 0.0f, static_cast<float>(kMaxXYValue)));
}

// The inverse. MoveToColor accepts any xy in the plane, including points outside the sRGB triangle that
// have no hue/saturation at all, so the out-of-gamut result is brought back in rather than trusted.
void ColorConverter::ConvertXYToHueSat(uint16_t x, uint16_t y, uint8_t & outHue, uint8_t & outSat)
{
    const float chromaticityX = static_cast<float>(x) / kXYScale;
    const float chromaticityY = static_cast<float>(y) / kXYScale;

    // y == 0 is a legal CurrentY with no XYZ preimage (luminance divides by it). Report the unsaturated
    // white the cluster defaults to rather than inventing a hue.
    if (chromaticityY < kMinChromaticityY)
    {
        outHue = 0;
        outSat = 0;
        return;
    }

    // Normalize at Y == 1; only the ratios survive into hue/saturation.
    const float cieX = chromaticityX / chromaticityY;
    const float cieZ = (1.0f - chromaticityX - chromaticityY) / chromaticityY;

    float red   = (3.2404542f * cieX) - 1.5371385f - (0.4985314f * cieZ);
    float green = (-0.9692660f * cieX) + 1.8760108f + (0.0415560f * cieZ);
    float blue  = (0.0556434f * cieX) - 0.2040259f + (1.0572252f * cieZ);

    // Out of gamut: clip the negative channels, then rescale so the brightest channel is 1. Rescaling is
    // what makes this a pure chromaticity answer — any common factor is Level Control's business.
    red   = std::max(red, 0.0f);
    green = std::max(green, 0.0f);
    blue  = std::max(blue, 0.0f);

    const float peak = std::max({ red, green, blue });
    if (peak <= 0.0f)
    {
        outHue = 0;
        outSat = 0;
        return;
    }
    red   = SrgbFromLinear(red / peak);
    green = SrgbFromLinear(green / peak);
    blue  = SrgbFromLinear(blue / peak);

    // sRGB to HSV. `high` is 1 by construction after the rescale above, so saturation is the channel spread.
    const float high  = std::max({ red, green, blue });
    const float low   = std::min({ red, green, blue });
    const float range = high - low;

    float sector = 0.0f;
    if (range > 0.0f)
    {
        if (high == red)
        {
            sector = std::fmod(((green - blue) / range) + kHueSectors, kHueSectors);
        }
        else if (high == green)
        {
            sector = ((blue - red) / range) + 2.0f;
        }
        else
        {
            sector = ((red - green) / range) + 4.0f;
        }
    }

    const float saturation = (high <= 0.0f) ? 0.0f : (range / high);

    outHue = static_cast<uint8_t>(std::clamp((sector / kHueSectors) * kMaxHueSatValue, 0.0f, kMaxHueSatValue));
    outSat = static_cast<uint8_t>(std::clamp(saturation * kMaxHueSatValue, 0.0f, kMaxHueSatValue));
}

// The two hue/sat <-> mireds routes are compositions, not separate approximations: going through xy is
// what keeps a mode switch HS -> CT -> HS consistent with HS -> XY -> CT.
void ColorConverter::ConvertHueSatToMireds(uint8_t hue, uint8_t sat, uint16_t & outMireds)
{
    uint16_t x = 0;
    uint16_t y = 0;
    ConvertHueSatToXY(hue, sat, x, y);
    ConvertXYToMireds(x, y, outMireds);
}

void ColorConverter::ConvertMiredsToHueSat(uint16_t mireds, uint8_t & outHue, uint8_t & outSat)
{
    uint16_t x = 0;
    uint16_t y = 0;
    ConvertMiredsToXY(mireds, x, y);
    ConvertXYToHueSat(x, y, outHue, outSat);
}

} // namespace app
} // namespace chip
