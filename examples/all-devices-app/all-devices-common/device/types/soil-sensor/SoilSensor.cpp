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

const DataModel::DeviceTypeEntry kSoilSensorDeviceTypes[] = {
    Device::Type::kSoilSensor,
    Device::Type::kPowerSource,
};

} // namespace

SoilMoistureMeasurementLimits::TypeInfo::Type SoilSensor::DefaultSoilMoistureMeasurementLimits()
{
    static const Globals::Structs::MeasurementAccuracyRangeStruct::Type kAccuracyRange[] = {
        []() {
            Globals::Structs::MeasurementAccuracyRangeStruct::Type range = {};
            range.rangeMin   = 0;
            range.rangeMax   = 100;
            range.percentMax = MakeOptional(static_cast<chip::Percent100ths>(10));
            return range;
        }()
    };
    return {
        .measurementType  = Globals::MeasurementTypeEnum::kSoilMoisture,
        .measured         = true,
        .minMeasuredValue = 0,
        .maxMeasuredValue = 100,
        .accuracyRanges   = DataModel::List<const Globals::Structs::MeasurementAccuracyRangeStruct::Type>(kAccuracyRange),
    };
}

TemperatureMeasurementCluster::StartupConfiguration SoilSensor::DefaultTemperatureConfiguration()
{
    return {
        .minMeasuredValue = DataModel::MakeNullable(static_cast<int16_t>(-1000)),
        .maxMeasuredValue = DataModel::MakeNullable(static_cast<int16_t>(5000)),
        .tolerance        = 0,
    };
}

SoilSensor::SoilSensor(TimerDelegate & timerDelegate, bool includeTemperature) :
    SoilSensor(timerDelegate, DefaultSoilMoistureMeasurementLimits(),
               includeTemperature ? std::make_optional(DefaultTemperatureConfiguration()) : std::nullopt)
{}

SoilSensor::SoilSensor(TimerDelegate & timerDelegate, SoilMoistureMeasurementLimits::TypeInfo::Type moistureLimits,
                       std::optional<TemperatureMeasurementCluster::StartupConfiguration> tempConfig) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(kSoilSensorDeviceTypes)),
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

    // Create the soil measurement cluster with initial 50% reading.
    mSoilMeasurementCluster.Create(endpoint, mMoistureLimits);
    ReturnErrorOnFailure(mSoilMeasurementCluster.Cluster().SetSoilMoistureMeasuredValue(
        DataModel::MakeNullable(static_cast<chip::Percent>(50))));
    ReturnErrorOnFailure(provider.AddCluster(mSoilMeasurementCluster.Registration()));

    // Create the optional temperature measurement cluster with initial 21.00 deg C reading.
    if (mTempConfig.has_value())
    {
        TemperatureMeasurementCluster::OptionalAttributeSet optionalAttributeSet{ 0 };
        mTemperatureMeasurementCluster.Create(endpoint, optionalAttributeSet, *mTempConfig);
        ReturnErrorOnFailure(mTemperatureMeasurementCluster.Cluster().SetMeasuredValue(
            DataModel::MakeNullable(static_cast<int16_t>(2100))));
        ReturnErrorOnFailure(provider.AddCluster(mTemperatureMeasurementCluster.Registration()));
    }

    // Create the power source cluster with initial 100% battery reading.
    constexpr auto BatPercentRemainingId = Clusters::PowerSource::Attributes::BatPercentRemaining::Id;
    SimpleBatteryPowerSourceCluster::Config powerConfig("Soil Sensor Battery"_span,
                                                        Clusters::PowerSource::BatReplaceabilityEnum::kUserReplaceable,
                                                        mTimerDelegate);
    powerConfig.usedOptionalAttributes.Set<BatPercentRemainingId>();
    powerConfig.status = Clusters::PowerSource::PowerSourceStatusEnum::kActive;
    powerConfig.order  = 0;
    powerConfig.batPercentRemaining.SetNonNull(200); // 100% (doubled percentage)
    mEndpointList[0] = endpoint;

    mBatteryPowerSourceCluster.Create(endpoint, powerConfig);
    ReturnErrorOnFailure(mBatteryPowerSourceCluster.Cluster().SetEndpointList(Span<const EndpointId>(mEndpointList)));
    ReturnErrorOnFailure(provider.AddCluster(mBatteryPowerSourceCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void SoilSensor::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mBatteryPowerSourceCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mBatteryPowerSourceCluster.Cluster()));
        mBatteryPowerSourceCluster.Destroy();
    }
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
