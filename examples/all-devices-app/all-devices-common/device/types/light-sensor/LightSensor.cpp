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
#include <device/types/light-sensor/LightSensor.h>
#include <devices/Types.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

namespace {

constexpr uint16_t kDefaultMinLight       = 1;
constexpr uint16_t kDefaultMaxLight       = 10000;
constexpr uint16_t kDefaultLightTolerance = 100; // 1% tolerance

const IlluminanceMeasurementCluster::StartupConfiguration kDefaultLightConfig = {
    .minMeasuredValue = DataModel::MakeNullable<uint16_t>(kDefaultMinLight),
    .maxMeasuredValue = DataModel::MakeNullable<uint16_t>(kDefaultMaxLight),
    .tolerance        = kDefaultLightTolerance,
    .lightSensorType  = DataModel::MakeNullable<IlluminanceMeasurement::LightSensorTypeEnum>(
        IlluminanceMeasurement::LightSensorTypeEnum::kPhotodiode),
};

IlluminanceMeasurementCluster::OptionalAttributeSet DefaultOptionalAttributes()
{
    IlluminanceMeasurementCluster::OptionalAttributeSet optionalAttributes;
    optionalAttributes.Set<IlluminanceMeasurement::Attributes::Tolerance::Id>();
    optionalAttributes.Set<IlluminanceMeasurement::Attributes::LightSensorType::Id>();
    return optionalAttributes;
}

} // namespace

LightSensor::LightSensor(TimerDelegate & timerDelegate) :
    LightSensor(timerDelegate, kDefaultLightConfig, DefaultOptionalAttributes())
{}

LightSensor::LightSensor(TimerDelegate & timerDelegate, IlluminanceMeasurementCluster::StartupConfiguration lightConfig,
                         IlluminanceMeasurementCluster::OptionalAttributeSet optionalAttributes) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kLightSensor, 1)),
    mTimerDelegate(timerDelegate), mLightConfig(lightConfig), mOptionalAttributes(optionalAttributes)
{}

CHIP_ERROR LightSensor::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mIlluminanceMeasurementCluster.Create(endpoint, mOptionalAttributes, mLightConfig);
    ReturnErrorOnFailure(provider.AddCluster(mIlluminanceMeasurementCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void LightSensor::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mIlluminanceMeasurementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIlluminanceMeasurementCluster.Cluster()));
        mIlluminanceMeasurementCluster.Destroy();
    }
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

} // namespace app
} // namespace chip
