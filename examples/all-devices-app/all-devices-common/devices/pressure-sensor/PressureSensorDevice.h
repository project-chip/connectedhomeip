/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    you may obtain a copy of the License at
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
#include <app/clusters/pressure-measurement-server/PressureMeasurementCluster.h>
#include <devices/interface/SingleEndpointDevice.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

/// A base device type implementation for a Matter Pressure Sensor, supporting the
/// Pressure Measurement server cluster.
class PressureSensorDevice : public SingleEndpointDevice
{
public:
    PressureSensorDevice(TimerDelegate & timerDelegate, Clusters::PressureMeasurementCluster::Config pressureConfig);
    ~PressureSensorDevice() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Public getters for programmatic control
    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }

    Clusters::PressureMeasurementCluster & PressureMeasurementCluster() { return mPressureMeasurementCluster.Cluster(); }

protected:
    TimerDelegate & mTimerDelegate;
    const Clusters::PressureMeasurementCluster::Config mPressureConfig;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::PressureMeasurementCluster> mPressureMeasurementCluster;
};

} // namespace app
} // namespace chip
