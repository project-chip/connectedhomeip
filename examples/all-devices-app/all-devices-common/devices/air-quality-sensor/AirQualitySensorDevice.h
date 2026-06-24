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

#include <app/clusters/air-quality-server/AirQualityCluster.h>
#include <app/clusters/concentration-measurement-server/ConcentrationMeasurementCluster.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <devices/interface/SingleEndpointDevice.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

class AirQualitySensorDevice : public SingleEndpointDevice
{
public:
    using ConcentrationCluster = Clusters::ConcentrationMeasurement::ConcentrationMeasurementCluster;

    struct Config
    {
        BitFlags<Clusters::AirQuality::Feature> airQualityFeatures;
        ConcentrationCluster::Config co2Config;
    };

    AirQualitySensorDevice(TimerDelegate & timerDelegate, const Config & config);
    ~AirQualitySensorDevice() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointId parentId = kInvalidEndpointId) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::AirQualityCluster & AirQualityCluster();
    ConcentrationCluster & CO2Cluster();

protected:
    TimerDelegate & mTimerDelegate;
    Config mConfig;

    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::AirQualityCluster> mAirQualityCluster;
    LazyRegisteredServerCluster<ConcentrationCluster> mCO2Cluster;
};

} // namespace app
} // namespace chip
