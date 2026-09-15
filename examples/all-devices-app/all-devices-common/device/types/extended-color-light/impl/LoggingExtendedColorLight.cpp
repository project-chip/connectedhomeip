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

#include <device/types/extended-color-light/impl/LoggingExtendedColorLight.h>
#include <devices/Types.h>

#include <clusters/ColorControl/Enums.h>

namespace chip {
namespace app {

LoggingExtendedColorLight::LoggingExtendedColorLight(const Context & context) :
    LoggingLightDriver(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kExtendedColorLight, 1), context,
                       Conformance{
                           .colorFeatures = BitMask<Clusters::ColorControl::Feature>(
                               Clusters::ColorControl::Feature::kXy, Clusters::ColorControl::Feature::kColorTemperature,
                               Clusters::ColorControl::Feature::kHueAndSaturation, Clusters::ColorControl::Feature::kEnhancedHue,
                               Clusters::ColorControl::Feature::kColorLoop),
                           .initialColor = Clusters::ColorControl::XYColor{},
                       })
{}

} // namespace app
} // namespace chip
