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

#include <device/types/air-quality-sensor/AirQualitySensor.h>

#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/concentration-measurement-server/ConcentrationMeasurementCluster.h>
#include <app/clusters/relative-humidity-measurement-server/RelativeHumidityMeasurementCluster.h>
#include <app/clusters/temperature-measurement-server/TemperatureMeasurementCluster.h>
#include <lib/support/BitFlags.h>
#include <lib/support/TimerDelegate.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace chip {
namespace app {

class SimulatedAirQualitySensor : public AirQualitySensor, public TimerContext
{
public:
    using ConcentrationCluster                        = Clusters::ConcentrationMeasurement::ConcentrationMeasurementCluster;
    static constexpr size_t kMaxConcentrationClusters = 10;
    static constexpr System::Clock::Seconds16 kDefaultUpdateInterval = System::Clock::Seconds16(10);

    struct Config
    {
        AirQualitySensor::Config baseConfig;
        std::optional<Clusters::TemperatureMeasurementCluster::StartupConfiguration> temperature;
        std::optional<Clusters::RelativeHumidityMeasurementCluster::Config> humidity;
        std::array<ConcentrationCluster::Config, kMaxConcentrationClusters> concentrationConfigs;
        size_t numConcentrationConfigs = 0;

        Config & WithAirQuality(BitFlags<Clusters::AirQuality::Feature> features);
        Config & WithTemperature(int16_t min = -4000, int16_t max = 8000);     // 0.01 deg C
        Config & WithRelativeHumidity(uint16_t min = 0, uint16_t max = 10000); // 0.01 %

        // Standard spec-compliant gases:
        Config & WithCarbonDioxide(float min = 0.0f, float max = 5000.0f);
        Config & WithPm25(float min = 0.0f, float max = 1000.0f);
        Config & WithTotalVolatileOrganicCompounds(float min = 0.0f, float max = 10000.0f);
        Config & WithCarbonMonoxide(float min = 0.0f, float max = 1000.0f);
        Config & WithNitrogenDioxide(float min = 0.0f, float max = 1000.0f);
        Config & WithOzone(float min = 0.0f, float max = 1000.0f);
        Config & WithFormaldehyde(float min = 0.0f, float max = 1000.0f);
        Config & WithPm1(float min = 0.0f, float max = 1000.0f);
        Config & WithPm10(float min = 0.0f, float max = 1000.0f);
        Config & WithRadon(float min = 0.0f, float max = 10000.0f);

        Config & WithConcentration(const ConcentrationCluster::Config & customConfig);
        Config & WithAllConcentrationClusters();
    };

    SimulatedAirQualitySensor(TimerDelegate & timerDelegate, const Config & config);
    explicit SimulatedAirQualitySensor(TimerDelegate & timerDelegate);
    ~SimulatedAirQualitySensor() override;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // TimerContext
    void TimerFired() override;

    // Public cluster accessors for optional clusters
    Clusters::TemperatureMeasurementCluster * TemperatureCluster();
    Clusters::RelativeHumidityMeasurementCluster * HumidityCluster();
    ConcentrationCluster * GetConcentrationCluster(ClusterId clusterId);
    ConcentrationCluster * CO2Cluster();

protected:
    CHIP_ERROR RegisterAdditionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider) override;
    void UnregisterAdditionalClusters(CodeDrivenDataModelProvider & provider) override;

private:
    Config mConfig;
    uint32_t mTickCount = 0;

    LazyRegisteredServerCluster<Clusters::TemperatureMeasurementCluster> mTemperatureCluster;
    LazyRegisteredServerCluster<Clusters::RelativeHumidityMeasurementCluster> mHumidityCluster;
    std::array<LazyRegisteredServerCluster<ConcentrationCluster>, kMaxConcentrationClusters> mConcentrationClusters;
    size_t mNumConcentrationClusters = 0;
};

} // namespace app
} // namespace chip
