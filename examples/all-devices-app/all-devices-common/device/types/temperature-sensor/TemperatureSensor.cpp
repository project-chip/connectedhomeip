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
#include <device/types/temperature-sensor/TemperatureSensor.h>
#include <devices/Types.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

namespace {

constexpr uint16_t kDefaultTemperatureTolerance = 50; // Set tolerance to 0.50 °C

const TemperatureMeasurementCluster::StartupConfiguration kDefaultTemperatureConfig = {
    .minMeasuredValue = DataModel::MakeNullable<int16_t>(-1000), // -10.00 °C
    .maxMeasuredValue = DataModel::MakeNullable<int16_t>(5000),  // +50.00 °C
    .tolerance        = kDefaultTemperatureTolerance,
};

TemperatureMeasurementCluster::OptionalAttributeSet DefaultOptionalAttributes()
{
    TemperatureMeasurementCluster::OptionalAttributeSet optionalAttributes;
    optionalAttributes.Set<TemperatureMeasurement::Attributes::Tolerance::Id>();
    return optionalAttributes;
}

} // namespace

TemperatureSensor::TemperatureSensor(TimerDelegate & timerDelegate) :
    TemperatureSensor(timerDelegate, kDefaultTemperatureConfig, DefaultOptionalAttributes())
{}

TemperatureSensor::TemperatureSensor(TimerDelegate & timerDelegate, TemperatureMeasurementCluster::StartupConfiguration tempConfig,
                                     TemperatureMeasurementCluster::OptionalAttributeSet optionalAttributes) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kTemperatureSensor, 1)),
    mTimerDelegate(timerDelegate), mTempConfig(tempConfig), mOptionalAttributes(optionalAttributes)
{}

CHIP_ERROR TemperatureSensor::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    // Create the identify cluster.
    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    // Create the temperature measurement cluster
    mTemperatureMeasurementCluster.Create(endpoint, mOptionalAttributes, mTempConfig);
    ReturnErrorOnFailure(provider.AddCluster(mTemperatureMeasurementCluster.Registration()));

    ReturnErrorOnFailure(RegisterAdditionalClusters(endpoint, provider));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void TemperatureSensor::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    UnregisterAdditionalClusters(provider);
    if (mTemperatureMeasurementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mTemperatureMeasurementCluster.Cluster()));
        mTemperatureMeasurementCluster.Destroy();
    }
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

} // namespace app
} // namespace chip
