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
 * Extended Color Light (device type 0x010D).
 *
 * Same cluster set as the Color Temperature Light, with a wider Color Control conformance: XY and
 * ColorTemperature are both mandatory here, and hue/saturation may be added. The feature tables
 * backing that live in ExtendedColorLight.cpp.
 */
class ExtendedColorLight : public ColorLight
{
public:
    explicit ExtendedColorLight(const Context & context);
    ~ExtendedColorLight() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;
};

} // namespace app
} // namespace chip
