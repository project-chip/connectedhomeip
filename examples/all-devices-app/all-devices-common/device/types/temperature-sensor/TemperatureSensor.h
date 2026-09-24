/*
 *
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
#include <app/clusters/temperature-measurement-server/TemperatureMeasurementCluster.h>
#include <app/clusters/thermostat-user-interface-configuration-server/ThermostatUserInterfaceConfigurationCluster.h>
#include <device/api/SingleEndpoint.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

class TemperatureSensor : public SingleEndpoint
{
public:
    TemperatureSensor(TimerDelegate & timerDelegate, Clusters::TemperatureMeasurementCluster::StartupConfiguration tempConfig,
                      Clusters::TemperatureMeasurementCluster::OptionalAttributeSet optionalAttributes = {});
    ~TemperatureSensor() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Public getters for programmatic control
    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }

    Clusters::TemperatureMeasurementCluster & TemperatureMeasurementCluster() { return mTemperatureMeasurementCluster.Cluster(); }

    Clusters::ThermostatUserInterfaceConfigurationCluster & ThermostatUserInterfaceConfigurationCluster()
    {
        return mUserInterfaceCluster.Cluster();
    }

protected:
    TimerDelegate & mTimerDelegate;
    Clusters::TemperatureMeasurementCluster::StartupConfiguration mTempConfig;
    Clusters::TemperatureMeasurementCluster::OptionalAttributeSet mOptionalAttributes;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::TemperatureMeasurementCluster> mTemperatureMeasurementCluster;
    LazyRegisteredServerCluster<Clusters::ThermostatUserInterfaceConfigurationCluster> mUserInterfaceCluster;
};

} // namespace app
} // namespace chip
