/*
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

#include <devices/cooktop/CooktopDevice.h>
#include <devices/interface/DeviceInterface.h>
#include <devices/temperature-controlled-cabinet/TemperatureControlledCabinetPart.h>

namespace chip::app {

class OvenDevice : public DeviceInterface
{
public:
    struct Config
    {
        TemperatureControlledCabinetPart::Config cavityConfig;

        // Sensible defaults representing a typical domestic oven:
        // - 180°C is the standard baking temperature (equivalent to ~350°F).
        // - 50°C to 250°C covers the typical domestic cooking range.
        // - 5°C steps are standard for digital oven controls.
        // Note: Temperature values are represented in 0.01°C steps.
        static constexpr Config Default()
        {
            return Config{ .cavityConfig = {
                               .temperatureSetpoint = 18000, // 180.00 °C
                               .minTemperature      = 5000,  // 50.00 °C
                               .maxTemperature      = 25000, // 250.00 °C
                               .step                = 500,   // 5.00 °C
                           } };
        }
    };

    OvenDevice(TimerDelegate & timerDelegate, Clusters::OnOffDelegate & surfaceOnOff, Clusters::IdentifyDelegate & cavityIdentify,
               Clusters::IdentifyDelegate & surfaceIdentify, const Config & config = Config::Default());
    ~OvenDevice() override = default;

    CHIP_ERROR Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Composition getters to expose child endpoints
    TemperatureControlledCabinetPart & Cavity() { return mCavity; }
    CookSurfacePart & Surface() { return mSurface; }

private:
    EndpointId mEndpointId = kInvalidEndpointId;
    TemperatureControlledCabinetPart mCavity;
    CookSurfacePart mSurface;
};

} // namespace chip::app
