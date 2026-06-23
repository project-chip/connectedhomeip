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
#include <app/clusters/illuminance-measurement-server/IlluminanceMeasurementCluster.h>
#include <devices/interface/SingleEndpointDevice.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

/// A base device type implementation for a Matter Light Sensor, supporting the
/// Illuminance Measurement server cluster.
class LightSensorDevice : public SingleEndpointDevice
{
public:
    LightSensorDevice(TimerDelegate & timerDelegate, Clusters::IlluminanceMeasurementCluster::StartupConfiguration lightConfig,
                      Clusters::IlluminanceMeasurementCluster::OptionalAttributeSet optionalAttributes = {});
    ~LightSensorDevice() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::IlluminanceMeasurementCluster & IlluminanceMeasurementCluster() { return mIlluminanceMeasurementCluster.Cluster(); }

protected:
    TimerDelegate & mTimerDelegate;
    const Clusters::IlluminanceMeasurementCluster::StartupConfiguration mLightConfig;
    const Clusters::IlluminanceMeasurementCluster::OptionalAttributeSet mOptionalAttributes;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::IlluminanceMeasurementCluster> mIlluminanceMeasurementCluster;
};

} // namespace app
} // namespace chip
