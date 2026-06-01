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
#include <devices/Types.h>
#include <devices/soil-sensor/SoilSensorDevice.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SoilMeasurement;
using namespace chip::app::Clusters::SoilMeasurement::Attributes;

namespace chip {
namespace app {

SoilSensorDevice::SoilSensorDevice(TimerDelegate & timerDelegate, SoilMoistureMeasurementLimits::TypeInfo::Type moistureLimits,
                                   TemperatureMeasurementCluster::StartupConfiguration tempConfig) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kSoilSensor, 1)),
    mTimerDelegate(timerDelegate), mMoistureLimits(moistureLimits), mTempConfig(tempConfig)
{}

CHIP_ERROR SoilSensorDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    ReturnErrorOnFailure(SingleEndpointRegistration(endpoint, provider, parentId));

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

    return provider.AddEndpoint(mEndpointRegistration);
}

void SoilSensorDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    SingleEndpointUnregistration(provider);
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
