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

#include <devices/interface/DeviceInterface.h>
#include <devices/temperature-controlled-cabinet/TemperatureControlledCabinetPart.h>

namespace chip::app {

class RefrigeratorDevice : public DeviceInterface
{
public:
    struct Config
    {
        TemperatureControlledCabinetPart::Config cabinetConfig;

        // Safe food storage defaults representing a typical domestic refrigerator:
        // - 4°C is the globally recommended safe temperature to prevent food spoilage.
        // - 1°C to 7°C represents the typical safe operating boundaries.
        // - 0.1°C steps provide precise temperature adjustment.
        // Note: Temperature values are represented in 0.01°C steps.
        static constexpr Config Default()
        {
            return Config{ .cabinetConfig = {
                               .temperatureSetpoint = 400, // 4.00 °C
                               .minTemperature      = 100, // 1.00 °C
                               .maxTemperature      = 700, // 7.00 °C
                               .step                = 10,  // 0.10 °C
                           } };
        }
    };

    RefrigeratorDevice(TimerDelegate & timerDelegate, Clusters::IdentifyDelegate & cabinetIdentify,
                       const Config & config = Config::Default());
    ~RefrigeratorDevice() override = default;

    CHIP_ERROR Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Composition getters to expose child endpoints
    TemperatureControlledCabinetPart & Cabinet() { return mCabinet; }

private:
    EndpointId mEndpointId = kInvalidEndpointId;
    TemperatureControlledCabinetPart mCabinet;
};

} // namespace chip::app
