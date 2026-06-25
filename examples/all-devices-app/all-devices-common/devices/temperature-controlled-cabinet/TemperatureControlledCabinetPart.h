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

#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/temperature-control-server/TemperatureControlCluster.h>
#include <devices/interface/SingleEndpointDevice.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app {

class TemperatureControlledCabinetPart : public SingleEndpointDevice
{
public:
    using SingleEndpointDevice::Register;

    struct Config
    {
        int16_t temperatureSetpoint = 400; // 4.00 °C
        int16_t minTemperature      = 100; // 1.00 °C
        int16_t maxTemperature      = 700; // 7.00 °C
        int16_t step                = 10;  // 0.10 °C
    };

    TemperatureControlledCabinetPart(TimerDelegate & timerDelegate, Clusters::IdentifyDelegate & identifyDelegate);
    TemperatureControlledCabinetPart(TimerDelegate & timerDelegate, Config config, Clusters::IdentifyDelegate & identifyDelegate);
    ~TemperatureControlledCabinetPart() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Public cluster getters for programmatic control
    Clusters::TemperatureControlCluster & TemperatureControlCluster() { return mTemperatureControlCluster.Cluster(); }
    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }

private:
    TimerDelegate & mTimerDelegate;
    const Config mConfig;
    Clusters::IdentifyDelegate & mIdentifyDelegate;

    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::TemperatureControlCluster> mTemperatureControlCluster;
};

} // namespace chip::app
