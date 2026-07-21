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
#include <app/clusters/relative-humidity-measurement-server/RelativeHumidityMeasurementCluster.h>
#include <device/api/SingleEndpoint.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

/// A base device type implementation for a Matter Humidity Sensor, supporting the
/// Relative Humidity Measurement server cluster.
class HumiditySensor : public SingleEndpoint
{
public:
    HumiditySensor(TimerDelegate & timerDelegate, Clusters::RelativeHumidityMeasurementCluster::Config humidityConfig);
    ~HumiditySensor() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Public getters for programmatic control
    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }

    Clusters::RelativeHumidityMeasurementCluster & RelativeHumidityMeasurementCluster()
    {
        return mRelativeHumidityMeasurementCluster.Cluster();
    }

protected:
    TimerDelegate & mTimerDelegate;
    const Clusters::RelativeHumidityMeasurementCluster::Config mHumidityConfig;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::RelativeHumidityMeasurementCluster> mRelativeHumidityMeasurementCluster;
};

} // namespace app
} // namespace chip
