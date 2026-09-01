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
 * The XY <-> color temperature conversions that a device advertising both features owes the
 * ColorControl cluster (§3.2.8.2), approximated against the Planckian locus in CIE 1931 xy.
 *
 * Without them the cluster has nothing to answer with: a read of ColorTemperatureMireds while the
 * endpoint is in XY mode leaves the out-param untouched and reports a constant, whatever the actual
 * color is. A real product implements these against its own LED calibration; this is the textbook
 * approximation, supplied so an example device reports a color temperature that tracks its xy.
 */
class PlanckianColorConverter : public Clusters::ColorControlDelegate
{
protected:
    void ConvertXYToMireds(uint16_t x, uint16_t y, uint16_t & outMireds) override;
    void ConvertMiredsToXY(uint16_t mireds, uint16_t & outX, uint16_t & outY) override;
};

} // namespace app
} // namespace chip
