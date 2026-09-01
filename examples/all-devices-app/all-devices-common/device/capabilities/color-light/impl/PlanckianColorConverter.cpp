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

#include <device/capabilities/color-light/impl/PlanckianColorConverter.h>

#include <algorithm>
#include <cmath>

namespace chip {
namespace app {
namespace {

// §3.2.7.9: CurrentX/CurrentY are the chromaticity coordinate scaled by 65536, constrained to
// 0xFEFF.
constexpr float kXYScale       = 65536.0f;
constexpr uint16_t kMaxXYValue = 0xFEFF;

// Neither approximation below is defined outside this window, and both diverge sharply past it, so
// every correlated color temperature is forced into it before use. That is also what bounds the
// mireds returned here to [40, 599]: the cluster encodes a delegate's result into
// ColorTemperatureMireds WITHOUT clamping it, so nothing else keeps the attribute inside its
// constraint. A product narrowing colorTempPhysicalMin/MaxMireds below that span has to narrow this
// window to match.
constexpr float kMinKelvin = 1667.0f;
constexpr float kMaxKelvin = 25000.0f;

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

} // namespace

// McCamy's cubic approximation of the correlated color temperature: the point on the Planckian
// locus closest to the given xy. It only carries meaning for an xy near that locus, and MoveToColor
// accepts any xy in the plane, so the result is constrained rather than trusted.
void PlanckianColorConverter::ConvertXYToMireds(uint16_t x, uint16_t y, uint16_t & outMireds)
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
void PlanckianColorConverter::ConvertMiredsToXY(uint16_t mireds, uint16_t & outX, uint16_t & outY)
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

} // namespace app
} // namespace chip
