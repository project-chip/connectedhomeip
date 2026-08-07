/**
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <cstdint>

namespace chip {
namespace app {
namespace Clusters {

// A delegate instance is bound to a single ColorControlCluster (and therefore a single endpoint).
// Matches it's siblings LevelControlDelegate / OnOffDelegate.
class ColorControlDelegate
{
public:
    virtual ~ColorControlDelegate() = default;

    // ---- Color mode conversion (§3.2.8.2) ----
    // Called by the cluster when a command switches ColorMode (e.g., XY → HS) and whenever an
    // inactive-mode attribute is read. The cluster passes the current color; the delegate fills
    // the requested representation in the out-params.
    //
    // For any device that advertises both features, this conversion is GUARANTEED to be
    // provided by the manufacturer both representations describe the same physical color,
    // so a mapping always exists.

    virtual void ConvertXYToHueSat(uint16_t x, uint16_t y, uint8_t & outHue, uint8_t & outSat) {}
    virtual void ConvertHueSatToXY(uint8_t hue, uint8_t sat, uint16_t & outX, uint16_t & outY) {}
    virtual void ConvertXYToMireds(uint16_t x, uint16_t y, uint16_t & outMireds) {}
    virtual void ConvertMiredsToXY(uint16_t mireds, uint16_t & outX, uint16_t & outY) {}
    virtual void ConvertHueSatToMireds(uint8_t hue, uint8_t sat, uint16_t & outMireds) {}
    virtual void ConvertMiredsToHueSat(uint16_t mireds, uint8_t & outHue, uint8_t & outSat) {}

    // ---- Hardware output (PWM, LEDs, etc.) ----
    // §3.2.8 requires the physical movement to be continuous, so the cluster feeds the value on every tick.
    virtual void OnColorXYChanged(uint16_t x, uint16_t y) {}
    virtual void OnColorHSChanged(uint8_t hue, uint8_t sat) {}
    virtual void OnColorCTChanged(uint16_t mireds) {}
    virtual void OnEnhancedHueChanged(uint16_t enhancedHue) {}
};

} // namespace Clusters
} // namespace app
} // namespace chip
