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
#include <devices/interface/SingleEndpointDevice.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

class TemperatureSensorDevice : public SingleEndpointDevice
{
public:
    TemperatureSensorDevice(TimerDelegate & timerDelegate,
                            Clusters::TemperatureMeasurementCluster::StartupConfiguration tempConfig);
    ~TemperatureSensorDevice() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointId parentId = kInvalidEndpointId) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::TemperatureMeasurementCluster & TemperatureMeasurementCluster() { return mTemperatureMeasurementCluster.Cluster(); }

protected:
    TimerDelegate & mTimerDelegate;
    Clusters::TemperatureMeasurementCluster::StartupConfiguration mTempConfig;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::TemperatureMeasurementCluster> mTemperatureMeasurementCluster;
};

} // namespace app
} // namespace chip
