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

#include <device/types/extended-color-light/ExtendedColorLight.h>
#include <devices/Types.h>

#include <clusters/ColorControl/Enums.h>

namespace chip {
namespace app {

// Color Control is the only cluster whose conformance is specific to device type 0x010D: the
// element requirements it shares with the Color Temperature Light are applied by
// ColorLight::Register() and documented there.
//
// Only the two mandatory features are advertised. Both being advertised, the cluster already asks
// the delegate to convert between XY and mireds on every mode switch and on every read of the
// inactive mode's attribute. LoggingLightDriver does not convert, so those report the target
// attribute's startup default instead of the active color; a product ships a calibration there.
// Adding HueSaturation / EnhancedHue / ColorLoop would add the remaining conversion pairs.
ExtendedColorLight::ExtendedColorLight(const Context & context) :
    LoggingLightDriver(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kExtendedColorLight, 1), context,
                       Conformance{
                           .colorFeatures = BitMask<Clusters::ColorControl::Feature>(
                               Clusters::ColorControl::Feature::kXy, Clusters::ColorControl::Feature::kColorTemperature),
                           // XY is the mode this device type powers up in.
                           .initialColor = Clusters::ColorControl::XYColor{},
                       })
{}

} // namespace app
} // namespace chip
