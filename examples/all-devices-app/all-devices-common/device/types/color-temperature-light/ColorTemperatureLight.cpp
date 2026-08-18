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

#include <device/types/color-temperature-light/ColorTemperatureLight.h>
#include <devices/Types.h>

#include <clusters/ColorControl/Enums.h>

namespace chip {
namespace app {

// Color Control is the only cluster whose conformance is specific to device type 0x010C: the
// element requirements it shares with the Extended Color Light are applied by ColorLight::Register()
// and documented there. ColorTemperature is the sole mandatory feature here, so it is also the mode
// the endpoint powers up in.
ColorTemperatureLight::ColorTemperatureLight(const Context & context) :
    LoggingLightDriver(
        Span<const DataModel::DeviceTypeEntry>(&Device::Type::kColorTemperatureLight, 1), context,
        Conformance{
            .colorFeatures = BitMask<Clusters::ColorControl::Feature>(Clusters::ColorControl::Feature::kColorTemperature),
            .initialColor  = Clusters::ColorControl::CTColor{},
        })
{}

} // namespace app
} // namespace chip
