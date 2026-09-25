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

#include "SimulatedAirQualitySensor.h"
#include <lib/support/CodeUtils.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

namespace {

using namespace chip::app::Clusters::ConcentrationMeasurement;

SimulatedAirQualitySensor::ConcentrationCluster::Config MakeGasConfig(ClusterId clusterId, MeasurementUnitEnum unit, float min, float max)
{
    return SimulatedAirQualitySensor::ConcentrationCluster::Config{
        .clusterId   = clusterId,
        .features    = BitFlags<Feature>(Feature::kNumericMeasurement, Feature::kPeakMeasurement, Feature::kAverageMeasurement,
                                      Feature::kLevelIndication),
        .medium      = MeasurementMediumEnum::kAir,
        .unit        = unit,
        .minMeasured = DataModel::MakeNullable(min),
        .maxMeasured = DataModel::MakeNullable(max),
    };
}

SimulatedAirQualitySensor::Config DefaultSimulatedConfig()
{
    SimulatedAirQualitySensor::Config config;
    config.WithTemperature().WithRelativeHumidity().WithCarbonDioxide();
    return config;
}

} // namespace

SimulatedAirQualitySensor::Config & SimulatedAirQualitySensor::Config::WithAirQuality(BitFlags<Clusters::AirQuality::Feature> features)
{
    baseConfig.WithAirQuality(features);
    return *this;
}

SimulatedAirQualitySensor::Config & SimulatedAirQualitySensor::Config::WithTemperature(int16_t min, int16_t max)
{
    temperature = Clusters::TemperatureMeasurementCluster::StartupConfiguration{
        .minMeasuredValue = DataModel::MakeNullable(min),
        .maxMeasuredValue = DataModel::MakeNullable(max),
    };
    return *this;
}

SimulatedAirQualitySensor::Config & SimulatedAirQualitySensor::Config::WithRelativeHumidity(uint16_t min, uint16_t max)
{
    Clusters::RelativeHumidityMeasurementCluster::Config cfg;
    cfg.minMeasuredValue = DataModel::MakeNullable(min);
    cfg.maxMeasuredValue = DataModel::MakeNullable(max);
    humidity             = cfg;
    return *this;
}

SimulatedAirQualitySensor::Config & SimulatedAirQualitySensor::Config::WithConcentration(const ConcentrationCluster::Config & customConfig)
{
    for (size_t i = 0; i < numConcentrationConfigs; ++i)
    {
        if (concentrationConfigs[i].clusterId == customConfig.clusterId)
        {
            concentrationConfigs[i] = customConfig;
            return *this;
        }
    }
    VerifyOrDie(numConcentrationConfigs < kMaxConcentrationClusters);
    concentrationConfigs[numConcentrationConfigs++] = customConfig;
    return *this;
}

SimulatedAirQualitySensor::Config & SimulatedAirQualitySensor::Config::WithCarbonDioxide(float min, float max)
{
    return WithConcentration(MakeGasConfig(CarbonDioxideConcentrationMeasurement::Id, MeasurementUnitEnum::kPpm, min, max));
}

SimulatedAirQualitySensor::Config & SimulatedAirQualitySensor::Config::WithPm25(float min, float max)
{
    return WithConcentration(MakeGasConfig(Pm25ConcentrationMeasurement::Id, MeasurementUnitEnum::kUgm3, min, max));
}

SimulatedAirQualitySensor::Config & SimulatedAirQualitySensor::Config::WithTotalVolatileOrganicCompounds(float min, float max)
{
    return WithConcentration(
        MakeGasConfig(TotalVolatileOrganicCompoundsConcentrationMeasurement::Id, MeasurementUnitEnum::kPpm, min, max));
}

SimulatedAirQualitySensor::Config & SimulatedAirQualitySensor::Config::WithCarbonMonoxide(float min, float max)
{
    return WithConcentration(MakeGasConfig(CarbonMonoxideConcentrationMeasurement::Id, MeasurementUnitEnum::kPpm, min, max));
}

SimulatedAirQualitySensor::Config & SimulatedAirQualitySensor::Config::WithNitrogenDioxide(float min, float max)
{
    return WithConcentration(MakeGasConfig(NitrogenDioxideConcentrationMeasurement::Id, MeasurementUnitEnum::kPpm, min, max));
}

SimulatedAirQualitySensor::Config & SimulatedAirQualitySensor::Config::WithOzone(float min, float max)
{
    return WithConcentration(MakeGasConfig(OzoneConcentrationMeasurement::Id, MeasurementUnitEnum::kPpm, min, max));
}

SimulatedAirQualitySensor::Config & SimulatedAirQualitySensor::Config::WithFormaldehyde(float min, float max)
{
    return WithConcentration(MakeGasConfig(FormaldehydeConcentrationMeasurement::Id, MeasurementUnitEnum::kPpm, min, max));
}

SimulatedAirQualitySensor::Config & SimulatedAirQualitySensor::Config::WithPm1(float min, float max)
{
    return WithConcentration(MakeGasConfig(Pm1ConcentrationMeasurement::Id, MeasurementUnitEnum::kUgm3, min, max));
}

SimulatedAirQualitySensor::Config & SimulatedAirQualitySensor::Config::WithPm10(float min, float max)
{
    return WithConcentration(MakeGasConfig(Pm10ConcentrationMeasurement::Id, MeasurementUnitEnum::kUgm3, min, max));
}

SimulatedAirQualitySensor::Config & SimulatedAirQualitySensor::Config::WithRadon(float min, float max)
{
    return WithConcentration(MakeGasConfig(RadonConcentrationMeasurement::Id, MeasurementUnitEnum::kBqm3, min, max));
}

SimulatedAirQualitySensor::Config & SimulatedAirQualitySensor::Config::WithAllConcentrationClusters()
{
    WithCarbonDioxide();
    WithPm25();
    WithTotalVolatileOrganicCompounds();
    WithCarbonMonoxide();
    WithNitrogenDioxide();
    WithOzone();
    WithFormaldehyde();
    WithPm1();
    WithPm10();
    WithRadon();
    return *this;
}

SimulatedAirQualitySensor::SimulatedAirQualitySensor(TimerDelegate & timerDelegate, const Config & config) :
    AirQualitySensor(timerDelegate, config.baseConfig), mConfig(config)
{}

SimulatedAirQualitySensor::SimulatedAirQualitySensor(TimerDelegate & timerDelegate) :
    AirQualitySensor(timerDelegate, DefaultSimulatedConfig().baseConfig), mConfig(DefaultSimulatedConfig())
{}

SimulatedAirQualitySensor::~SimulatedAirQualitySensor()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR SimulatedAirQualitySensor::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                               EndpointComposition composition)
{
    ReturnErrorOnFailure(AirQualitySensor::Register(endpoint, provider, composition));
    return mTimerDelegate.StartTimer(this, kDefaultUpdateInterval);
}

void SimulatedAirQualitySensor::Unregister(CodeDrivenDataModelProvider & provider)
{
    mTimerDelegate.CancelTimer(this);
    AirQualitySensor::Unregister(provider);
}

CHIP_ERROR SimulatedAirQualitySensor::RegisterAdditionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
{
    if (mConfig.temperature.has_value())
    {
        mTemperatureCluster.Create(endpoint, TemperatureMeasurementCluster::OptionalAttributeSet(), *mConfig.temperature);
        ReturnErrorOnFailure(provider.AddCluster(mTemperatureCluster.Registration()));
    }

    if (mConfig.humidity.has_value())
    {
        mHumidityCluster.Create(endpoint, *mConfig.humidity);
        ReturnErrorOnFailure(provider.AddCluster(mHumidityCluster.Registration()));
    }

    mNumConcentrationClusters = mConfig.numConcentrationConfigs;
    for (size_t i = 0; i < mNumConcentrationClusters; ++i)
    {
        mConcentrationClusters[i].Create(endpoint, mConfig.concentrationConfigs[i]);
        ReturnErrorOnFailure(provider.AddCluster(mConcentrationClusters[i].Registration()));
    }

    return CHIP_NO_ERROR;
}

void SimulatedAirQualitySensor::UnregisterAdditionalClusters(CodeDrivenDataModelProvider & provider)
{
    for (size_t i = 0; i < mNumConcentrationClusters; ++i)
    {
        if (mConcentrationClusters[i].IsConstructed())
        {
            LogErrorOnFailure(provider.RemoveCluster(&mConcentrationClusters[i].Cluster()));
            mConcentrationClusters[i].Destroy();
        }
    }
    mNumConcentrationClusters = 0;

    if (mHumidityCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mHumidityCluster.Cluster()));
        mHumidityCluster.Destroy();
    }
    if (mTemperatureCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mTemperatureCluster.Cluster()));
        mTemperatureCluster.Destroy();
    }
}

Clusters::TemperatureMeasurementCluster * SimulatedAirQualitySensor::TemperatureCluster()
{
    return mTemperatureCluster.IsConstructed() ? &mTemperatureCluster.Cluster() : nullptr;
}

Clusters::RelativeHumidityMeasurementCluster * SimulatedAirQualitySensor::HumidityCluster()
{
    return mHumidityCluster.IsConstructed() ? &mHumidityCluster.Cluster() : nullptr;
}

SimulatedAirQualitySensor::ConcentrationCluster * SimulatedAirQualitySensor::GetConcentrationCluster(ClusterId clusterId)
{
    for (size_t i = 0; i < mNumConcentrationClusters; ++i)
    {
        if (mConcentrationClusters[i].IsConstructed() && mConfig.concentrationConfigs[i].clusterId == clusterId)
        {
            return &mConcentrationClusters[i].Cluster();
        }
    }
    return nullptr;
}

SimulatedAirQualitySensor::ConcentrationCluster * SimulatedAirQualitySensor::CO2Cluster()
{
    return GetConcentrationCluster(CarbonDioxideConcentrationMeasurement::Id);
}

void SimulatedAirQualitySensor::TimerFired()
{
    mTickCount++;

    // 1. Advance Air Quality enum
    AirQuality::AirQualityEnum aqValue;
    switch (mTickCount % 3)
    {
    case 1:
        aqValue = AirQuality::AirQualityEnum::kGood;
        break;
    case 2:
        aqValue = AirQuality::AirQualityEnum::kFair;
        break;
    default:
        aqValue = AirQuality::AirQualityEnum::kModerate;
        break;
    }
    Protocols::InteractionModel::Status aqStatus = AirQualityCluster().SetAirQuality(aqValue);
    if (aqStatus != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(AppServer, "Failed to set air quality: %u", to_underlying(aqStatus));
    }

    // 2. Oscillate Temperature (~21.5°C ± 1.0°C)
    int16_t tempVal = static_cast<int16_t>(2150 + ((static_cast<int>(mTickCount) % 5) - 2) * 50);
    if (TemperatureCluster() != nullptr)
    {
        LogErrorOnFailure(TemperatureCluster()->SetMeasuredValue(DataModel::MakeNullable(tempVal)));
    }

    // 3. Oscillate Relative Humidity (~45% ± 3.0%)
    uint16_t humidityVal = static_cast<uint16_t>(4500 + ((static_cast<int>(mTickCount) % 5) - 2) * 150);
    if (HumidityCluster() != nullptr)
    {
        LogErrorOnFailure(HumidityCluster()->SetMeasuredValue(DataModel::MakeNullable(humidityVal)));
    }

    // 4. Oscillate CO2 (450 ppm to 850 ppm)
    float co2Val = 450.0f + static_cast<float>((mTickCount % 9) * 50);
    if (CO2Cluster() != nullptr)
    {
        LogErrorOnFailure(CO2Cluster()->SetMeasuredValue(DataModel::MakeNullable(co2Val)));
    }

    // 5. Update any other concentration clusters configured
    for (size_t i = 0; i < mNumConcentrationClusters; ++i)
    {
        if (mConcentrationClusters[i].IsConstructed())
        {
            ClusterId cid = mConfig.concentrationConfigs[i].clusterId;
            if (cid == CarbonDioxideConcentrationMeasurement::Id)
            {
                continue;
            }
            float val = 10.0f + static_cast<float>((mTickCount % 5) * 2);
            LogErrorOnFailure(mConcentrationClusters[i].Cluster().SetMeasuredValue(DataModel::MakeNullable(val)));
        }
    }

    ChipLogProgress(AppServer, "SimulatedAirQuality: AQ=%u, Temp=%.2fC, Humidity=%.1f%%, CO2=%.0fppm",
                    static_cast<unsigned>(aqValue), static_cast<double>(tempVal) / 100.0, static_cast<double>(humidityVal) / 100.0,
                    static_cast<double>(co2Val));

    LogErrorOnFailure(mTimerDelegate.StartTimer(this, kDefaultUpdateInterval));
}

} // namespace app
} // namespace chip
