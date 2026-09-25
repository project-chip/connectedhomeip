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

#include <device/types/air-quality-sensor/AirQualitySensor.h>
#include <devices/Types.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

namespace {

using namespace chip::app::Clusters::ConcentrationMeasurement;

AirQualitySensor::ConcentrationCluster::Config MakeGasConfig(ClusterId clusterId, MeasurementUnitEnum unit, float min, float max)
{
    return AirQualitySensor::ConcentrationCluster::Config{
        .clusterId   = clusterId,
        .features    = BitFlags<Feature>(Feature::kNumericMeasurement, Feature::kPeakMeasurement,
                                         Feature::kAverageMeasurement, Feature::kLevelIndication),
        .medium      = MeasurementMediumEnum::kAir,
        .unit        = unit,
        .minMeasured = DataModel::MakeNullable(min),
        .maxMeasured = DataModel::MakeNullable(max),
    };
}

} // namespace

AirQualitySensor::Config & AirQualitySensor::Config::WithAirQuality(BitFlags<Clusters::AirQuality::Feature> features)
{
    airQualityFeatures = features;
    return *this;
}

AirQualitySensor::Config & AirQualitySensor::Config::WithTemperature(int16_t min, int16_t max)
{
    temperature = Clusters::TemperatureMeasurementCluster::StartupConfiguration{
        .minMeasuredValue = DataModel::MakeNullable(min),
        .maxMeasuredValue = DataModel::MakeNullable(max),
    };
    return *this;
}

AirQualitySensor::Config & AirQualitySensor::Config::WithRelativeHumidity(uint16_t min, uint16_t max)
{
    Clusters::RelativeHumidityMeasurementCluster::Config cfg;
    cfg.minMeasuredValue = DataModel::MakeNullable(min);
    cfg.maxMeasuredValue = DataModel::MakeNullable(max);
    humidity             = cfg;
    return *this;
}

AirQualitySensor::Config & AirQualitySensor::Config::WithConcentration(const ConcentrationCluster::Config & customConfig)
{
    VerifyOrDie(numConcentrationConfigs < kMaxConcentrationClusters);
    concentrationConfigs[numConcentrationConfigs++] = customConfig;
    return *this;
}

AirQualitySensor::Config & AirQualitySensor::Config::WithCarbonDioxide(float min, float max)
{
    return WithConcentration(MakeGasConfig(CarbonDioxideConcentrationMeasurement::Id, MeasurementUnitEnum::kPpm, min, max));
}

AirQualitySensor::Config & AirQualitySensor::Config::WithPm25(float min, float max)
{
    return WithConcentration(MakeGasConfig(Pm25ConcentrationMeasurement::Id, MeasurementUnitEnum::kUgm3, min, max));
}

AirQualitySensor::Config & AirQualitySensor::Config::WithTotalVolatileOrganicCompounds(float min, float max)
{
    return WithConcentration(
        MakeGasConfig(TotalVolatileOrganicCompoundsConcentrationMeasurement::Id, MeasurementUnitEnum::kPpm, min, max));
}

AirQualitySensor::Config & AirQualitySensor::Config::WithCarbonMonoxide(float min, float max)
{
    return WithConcentration(MakeGasConfig(CarbonMonoxideConcentrationMeasurement::Id, MeasurementUnitEnum::kPpm, min, max));
}

AirQualitySensor::Config & AirQualitySensor::Config::WithNitrogenDioxide(float min, float max)
{
    return WithConcentration(MakeGasConfig(NitrogenDioxideConcentrationMeasurement::Id, MeasurementUnitEnum::kPpm, min, max));
}

AirQualitySensor::Config & AirQualitySensor::Config::WithOzone(float min, float max)
{
    return WithConcentration(MakeGasConfig(OzoneConcentrationMeasurement::Id, MeasurementUnitEnum::kPpm, min, max));
}

AirQualitySensor::Config & AirQualitySensor::Config::WithFormaldehyde(float min, float max)
{
    return WithConcentration(MakeGasConfig(FormaldehydeConcentrationMeasurement::Id, MeasurementUnitEnum::kPpm, min, max));
}

AirQualitySensor::Config & AirQualitySensor::Config::WithPm1(float min, float max)
{
    return WithConcentration(MakeGasConfig(Pm1ConcentrationMeasurement::Id, MeasurementUnitEnum::kUgm3, min, max));
}

AirQualitySensor::Config & AirQualitySensor::Config::WithPm10(float min, float max)
{
    return WithConcentration(MakeGasConfig(Pm10ConcentrationMeasurement::Id, MeasurementUnitEnum::kUgm3, min, max));
}

AirQualitySensor::Config & AirQualitySensor::Config::WithRadon(float min, float max)
{
    return WithConcentration(MakeGasConfig(RadonConcentrationMeasurement::Id, MeasurementUnitEnum::kBqm3, min, max));
}

AirQualitySensor::Config & AirQualitySensor::Config::WithAllConcentrationClusters()
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

AirQualitySensor::AirQualitySensor(TimerDelegate & timerDelegate, const Config & config) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kAirQualitySensor, 1)), mTimerDelegate(timerDelegate),
    mConfig(config)
{}

CHIP_ERROR AirQualitySensor::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                      EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mAirQualityCluster.Create(endpoint, mConfig.airQualityFeatures);
    ReturnErrorOnFailure(provider.AddCluster(mAirQualityCluster.Registration()));

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

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void AirQualitySensor::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);

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
    if (mAirQualityCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mAirQualityCluster.Cluster()));
        mAirQualityCluster.Destroy();
    }
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

Clusters::AirQualityCluster & AirQualitySensor::AirQualityCluster()
{
    VerifyOrDie(mAirQualityCluster.IsConstructed());
    return mAirQualityCluster.Cluster();
}

Clusters::TemperatureMeasurementCluster * AirQualitySensor::TemperatureCluster()
{
    return mTemperatureCluster.IsConstructed() ? &mTemperatureCluster.Cluster() : nullptr;
}

Clusters::RelativeHumidityMeasurementCluster * AirQualitySensor::HumidityCluster()
{
    return mHumidityCluster.IsConstructed() ? &mHumidityCluster.Cluster() : nullptr;
}

AirQualitySensor::ConcentrationCluster * AirQualitySensor::GetConcentrationCluster(ClusterId clusterId)
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

AirQualitySensor::ConcentrationCluster * AirQualitySensor::CO2Cluster()
{
    return GetConcentrationCluster(CarbonDioxideConcentrationMeasurement::Id);
}

} // namespace app
} // namespace chip
