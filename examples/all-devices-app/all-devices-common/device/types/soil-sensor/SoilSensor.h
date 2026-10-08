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

#include <optional>

#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/soil-measurement-server/SoilMeasurementCluster.h>
#include <app/clusters/temperature-measurement-server/TemperatureMeasurementCluster.h>
#include <device/api/SingleEndpoint.h>
#include <device/types/power-source/BatteryPowerSource.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

class SoilSensor : public SingleEndpoint
{
public:
    using SimpleBatteryPowerSourceCluster = BatteryPowerSource::SimpleBatteryPowerSourceCluster;

    static Clusters::SoilMeasurement::Attributes::SoilMoistureMeasurementLimits::TypeInfo::Type DefaultSoilMoistureMeasurementLimits();
    static Clusters::TemperatureMeasurementCluster::StartupConfiguration DefaultTemperatureConfiguration();

    SoilSensor(TimerDelegate & timerDelegate, bool includeTemperature = true);
    SoilSensor(TimerDelegate & timerDelegate,
               Clusters::SoilMeasurement::Attributes::SoilMoistureMeasurementLimits::TypeInfo::Type moistureLimits,
               std::optional<Clusters::TemperatureMeasurementCluster::StartupConfiguration> tempConfig = std::nullopt);
    ~SoilSensor() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Public getters for programmatic control
    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }
    Clusters::SoilMeasurementCluster & SoilMeasurementCluster() { return mSoilMeasurementCluster.Cluster(); }

    bool HasTemperature() const { return mTemperatureMeasurementCluster.IsConstructed(); }
    Clusters::TemperatureMeasurementCluster & TemperatureMeasurementCluster()
    {
        VerifyOrDie(mTemperatureMeasurementCluster.IsConstructed());
        return mTemperatureMeasurementCluster.Cluster();
    }

    SimpleBatteryPowerSourceCluster & PowerSourceCluster() { return mBatteryPowerSourceCluster.Cluster(); }

protected:
    TimerDelegate & mTimerDelegate;
    Clusters::SoilMeasurement::Attributes::SoilMoistureMeasurementLimits::TypeInfo::Type mMoistureLimits;
    std::optional<Clusters::TemperatureMeasurementCluster::StartupConfiguration> mTempConfig;
    EndpointId mEndpointList[1] = { kInvalidEndpointId };

    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::SoilMeasurementCluster> mSoilMeasurementCluster;
    LazyRegisteredServerCluster<Clusters::TemperatureMeasurementCluster> mTemperatureMeasurementCluster;
    LazyRegisteredServerCluster<SimpleBatteryPowerSourceCluster> mBatteryPowerSourceCluster;
};

} // namespace app
} // namespace chip
