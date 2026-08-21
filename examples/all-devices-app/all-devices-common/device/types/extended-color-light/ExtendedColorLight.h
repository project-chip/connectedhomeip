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

#include <device/capabilities/color-light/impl/LoggingLightDriver.h>

namespace chip {
namespace app {

/**
 * Extended Color Light (device type 0x010D).
 *
 * Registers the ColorLight cluster set with the conformance this device type requires. Everything
 * it shares with the Color Temperature Light is applied by ColorLight::Register(); what this device
 * type adds - Color Control with both XY and ColorTemperature - is the Conformance passed from
 * ExtendedColorLight.cpp.
 */
class ExtendedColorLight : public LoggingLightDriver
{
public:
    explicit ExtendedColorLight(const Context & context);
    ~ExtendedColorLight() override = default;
};

} // namespace app
} // namespace chip
