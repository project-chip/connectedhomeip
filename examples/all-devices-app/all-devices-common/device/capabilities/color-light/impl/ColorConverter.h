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

#pragma once

#include <app/clusters/color-control-server/ColorControlDelegate.h>

#include <cstdint>

namespace chip {
namespace app {

/**
 * Every pairwise conversion between the three color representations a device may advertise — XY,
 * color temperature and hue/saturation — which §3.2.8.2 makes it owe the ColorControl cluster once it
 * advertises more than one of them.
 *
 * Without them the cluster has nothing to answer with: a read of ColorTemperatureMireds while the
 * endpoint is in XY mode leaves the out-param untouched and reports a constant, whatever the actual
 * color is. A real product implements these against its own LED calibration; these are the textbook
 * approximations, supplied so an example device reports each mode tracking the others.
 *
 * CIE 1931 xy is the hub, so there are only two approximations rather than six: the Planckian locus
 * for color temperature (McCamy going in, Kim et al. coming back) and sRGB for hue/saturation. The
 * hue/sat <-> mireds pair is their composition, which is what keeps the routes mutually consistent.
 */
class ColorConverter : public Clusters::ColorControlDelegate
{
protected:
    void ConvertXYToMireds(uint16_t x, uint16_t y, uint16_t & outMireds) override;
    void ConvertMiredsToXY(uint16_t mireds, uint16_t & outX, uint16_t & outY) override;

    // Hue/saturation is a third representation of the same color, so the cluster needs it paired with
    // BOTH of the above. Only the xy pair is primitive: hue/sat travels through xy via sRGB, which keeps
    // every route between the three modes consistent with the two functions above.
    void ConvertXYToHueSat(uint16_t x, uint16_t y, uint8_t & outHue, uint8_t & outSat) override;
    void ConvertHueSatToXY(uint8_t hue, uint8_t sat, uint16_t & outX, uint16_t & outY) override;
    void ConvertHueSatToMireds(uint8_t hue, uint8_t sat, uint16_t & outMireds) override;
    void ConvertMiredsToHueSat(uint16_t mireds, uint8_t & outHue, uint8_t & outSat) override;
};

} // namespace app
} // namespace chip
