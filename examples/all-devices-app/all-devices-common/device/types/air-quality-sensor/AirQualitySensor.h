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

#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/air-quality-server/AirQualityCluster.h>
#include <app/clusters/concentration-measurement-server/ConcentrationMeasurementCluster.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/relative-humidity-measurement-server/RelativeHumidityMeasurementCluster.h>
#include <app/clusters/temperature-measurement-server/TemperatureMeasurementCluster.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/api/SingleEndpoint.h>
#include <lib/support/BitFlags.h>
#include <lib/support/TimerDelegate.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace chip {
namespace app {

/// Base Matter Air Quality Sensor device type (spec section 2.6).
/// Owns mandatory clusters (Identify, Air Quality) and configurable optional clusters
/// (Temperature, Relative Humidity, and Concentration measurements).
class AirQualitySensor : public SingleEndpoint
{
public:
    using ConcentrationCluster                        = Clusters::ConcentrationMeasurement::ConcentrationMeasurementCluster;
    static constexpr size_t kMaxConcentrationClusters = 10;

    enum class ConcentrationType : uint8_t
    {
        kCarbonDioxide,
        kPm25,
        kTotalVolatileOrganicCompounds,
        kCarbonMonoxide,
        kNitrogenDioxide,
        kOzone,
        kFormaldehyde,
        kPm1,
        kPm10,
        kRadon,
    };

    struct Config
    {
        BitFlags<Clusters::AirQuality::Feature> airQualityFeatures{ Clusters::AirQuality::Feature::kFair,
                                                                    Clusters::AirQuality::Feature::kModerate,
                                                                    Clusters::AirQuality::Feature::kVeryPoor,
                                                                    Clusters::AirQuality::Feature::kExtremelyPoor };
        std::optional<Clusters::TemperatureMeasurementCluster::StartupConfiguration> temperature;
        std::optional<Clusters::RelativeHumidityMeasurementCluster::Config> humidity;
        std::array<ConcentrationCluster::Config, kMaxConcentrationClusters> concentrationConfigs;
        size_t numConcentrationConfigs = 0;

        constexpr Config() = default;

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

    AirQualitySensor(TimerDelegate & timerDelegate, const Config & config);
    explicit AirQualitySensor(TimerDelegate & timerDelegate);
    ~AirQualitySensor() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Public cluster accessors for mandatory clusters
    Clusters::AirQualityCluster & AirQualityCluster();
    Clusters::IdentifyCluster & IdentifyCluster();

    // Public cluster accessors for optional clusters (returns nullptr if not configured)
    Clusters::TemperatureMeasurementCluster * TemperatureCluster();
    Clusters::RelativeHumidityMeasurementCluster * HumidityCluster();
    ConcentrationCluster * GetConcentrationCluster(ConcentrationType type);

protected:
    /// Called before the endpoint is added, within the registration transaction.
    /// Subclasses in impl/ override this to attach additional custom/vendor clusters.
    virtual CHIP_ERROR RegisterAdditionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
    {
        return CHIP_NO_ERROR;
    }

    /// Called after the endpoint is removed, also on partial registration failure.
    virtual void UnregisterAdditionalClusters(CodeDrivenDataModelProvider & provider) {}

    ConcentrationCluster * GetConcentrationCluster(ClusterId clusterId);

    TimerDelegate & mTimerDelegate;
    Config mConfig;

    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::AirQualityCluster> mAirQualityCluster;
    LazyRegisteredServerCluster<Clusters::TemperatureMeasurementCluster> mTemperatureCluster;
    LazyRegisteredServerCluster<Clusters::RelativeHumidityMeasurementCluster> mHumidityCluster;
    std::array<LazyRegisteredServerCluster<ConcentrationCluster>, kMaxConcentrationClusters> mConcentrationClusters;
    size_t mNumConcentrationClusters = 0;
};

} // namespace app
} // namespace chip
