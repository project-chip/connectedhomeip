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
#include <variant>

// Shared color-state vocabulary for the ColorControl cluster.
//
// These are the types that cross the cluster <-> scene-handler boundary: the active color value
// and the color-loop state. The per-axis TRANSITION structs are deliberately NOT here — they are
// cluster-internal and stay private to ColorControlCluster.

namespace chip::app::Clusters::ColorControl {


// CIE xy chromaticity coordinates.
struct XYColor
{
    uint16_t x = 0x616B; // ZAP defaults
    uint16_t y = 0x607D;
};
// Color Temperature in Mireds.
struct CTColor
{
    uint16_t mireds = 0x00FA;
};

// Legacy hue/sat (mode 0) — hue stored at native 8-bit precision.
struct HueSatColor
{
    uint8_t hue        = 0;
    uint8_t saturation = 0;

    // §3.2.7.12: EnhancedCurrentHue, when advertised in this mode, is the 8-bit hue in the high byte.
    uint16_t enhancedHue() const { return static_cast<uint16_t>(static_cast<uint16_t>(hue) << 8); }
};

// Enhanced hue/sat (mode 3) — hue stored at full 16-bit precision.
struct EnhancedHueSatColor
{
    uint16_t enhancedHue = 0;
    uint8_t  saturation  = 0;

    // §3.2.7.12: CurrentHue is the most-significant byte of EnhancedCurrentHue.
    uint8_t hue() const { return static_cast<uint8_t>(enhancedHue >> 8); }
};

// The single active color value. The alternative held encodes the (Enhanced)ColorMode, so
// colorMode / enhancedColorMode are derived from index() and never stored separately.
using ColorValue = std::variant<XYColor, HueSatColor, EnhancedHueSatColor, CTColor>;

// Mode-independent color-loop state (persistent). Kept out of the transition variant because the
// loop can be active-but-dormant while XY/CT owns the output.
struct ColorLoopState
{
    uint8_t  active            = 0;
    uint8_t  direction         = 0;
    uint16_t time              = 0x0019;
    uint16_t startEnhancedHue  = 0x2300;
    uint16_t storedEnhancedHue = 0;
};

} // namespace chip::app::Clusters::ColorControl
