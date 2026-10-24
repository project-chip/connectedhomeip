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
#include <device/types/soil-sensor/SoilSensor.h>
#include <devices/Types.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SoilMeasurement;
using namespace chip::app::Clusters::SoilMeasurement::Attributes;

namespace chip {
namespace app {

namespace {
constexpr chip::Percent kMinMoisturePercent = 0;
constexpr chip::Percent kMaxMoisturePercent = 100;

constexpr int16_t kMinTemperatureCentiCelsius = -1000; // -10.00°C
constexpr int16_t kMaxTemperatureCentiCelsius = 5000;  // +50.00°C

const Globals::Structs::MeasurementAccuracyRangeStruct::Type kDefaultSoilMoistureMeasurementLimitsAccuracyRange[] = {
    { .rangeMin   = kMinMoisturePercent,
      .rangeMax   = kMaxMoisturePercent,
      .percentMax = MakeOptional(static_cast<chip::Percent100ths>(10)) }
};

const SoilMoistureMeasurementLimits::TypeInfo::Type kDefaultSoilMoistureMeasurementLimits = {
    .measurementType  = Globals::MeasurementTypeEnum::kSoilMoisture,
    .measured         = true,
    .minMeasuredValue = kMinMoisturePercent,
    .maxMeasuredValue = kMaxMoisturePercent,
    .accuracyRanges   = DataModel::List<const Globals::Structs::MeasurementAccuracyRangeStruct::Type>(
        kDefaultSoilMoistureMeasurementLimitsAccuracyRange)
};

const TemperatureMeasurementCluster::StartupConfiguration kDefaultTemperatureConfig = {
    .minMeasuredValue = DataModel::MakeNullable(kMinTemperatureCentiCelsius),
    .maxMeasuredValue = DataModel::MakeNullable(kMaxTemperatureCentiCelsius),
    .tolerance        = 0,
};
} // namespace

SoilSensor::SoilSensor(TimerDelegate & timerDelegate) :
    SoilSensor(timerDelegate, kDefaultSoilMoistureMeasurementLimits, kDefaultTemperatureConfig)
{}

SoilSensor::SoilSensor(TimerDelegate & timerDelegate, SoilMoistureMeasurementLimits::TypeInfo::Type moistureLimits,
                       TemperatureMeasurementCluster::StartupConfiguration tempConfig) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kSoilSensor, 1)),
    mTimerDelegate(timerDelegate), mMoistureLimits(moistureLimits), mTempConfig(tempConfig)
{}

CHIP_ERROR SoilSensor::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    // Create the identify cluster.
    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    // Create the temperature measurement cluster
    TemperatureMeasurementCluster::OptionalAttributeSet optionalAttributeSet{ 0 };
    mTemperatureMeasurementCluster.Create(endpoint, optionalAttributeSet, mTempConfig);
    ReturnErrorOnFailure(provider.AddCluster(mTemperatureMeasurementCluster.Registration()));

    // Create the soil measurement cluster.
    mSoilMeasurementCluster.Create(endpoint, mMoistureLimits);
    ReturnErrorOnFailure(provider.AddCluster(mSoilMeasurementCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void SoilSensor::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mTemperatureMeasurementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mTemperatureMeasurementCluster.Cluster()));
        mTemperatureMeasurementCluster.Destroy();
    }
    if (mSoilMeasurementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mSoilMeasurementCluster.Cluster()));
        mSoilMeasurementCluster.Destroy();
    }
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

} // namespace app
} // namespace chip
