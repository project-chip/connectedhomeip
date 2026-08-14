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

#include <device/capabilities/color-light/ColorLight.h>

namespace chip {
namespace app {

/**
 * Color Temperature Light (device type 0x010C).
 *
 * Registers the ColorLight cluster set with the conformance this device type requires: On/Off
 * with Lighting, Level Control with OnOff + Lighting, and Color Control with ColorTemperature
 * only. The feature tables backing those live in ColorTemperatureLight.cpp.
 */
class ColorTemperatureLight : public ColorLight
{
public:
    explicit ColorTemperatureLight(const Context & context);
    ~ColorTemperatureLight() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;
};

} // namespace app
} // namespace chip
