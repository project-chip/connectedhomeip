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
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

AirQualitySensor::AirQualitySensor(TimerDelegate & timerDelegate, const Config & config) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kAirQualitySensor, 1)), mTimerDelegate(timerDelegate),
    mConfig(config)
{}

CHIP_ERROR AirQualitySensor::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                      EndpointComposition composition)
{
    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mAirQualityCluster.Create(endpoint, mConfig.airQualityFeatures);
    ReturnErrorOnFailure(provider.AddCluster(mAirQualityCluster.Registration()));

    mCO2Cluster.Create(endpoint, mConfig.co2Config);
    ReturnErrorOnFailure(provider.AddCluster(mCO2Cluster.Registration()));

    return provider.AddEndpoint(mEndpointRegistration);
}

void AirQualitySensor::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mCO2Cluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mCO2Cluster.Cluster()));
        mCO2Cluster.Destroy();
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

AirQualitySensor::ConcentrationCluster & AirQualitySensor::CO2Cluster()
{
    VerifyOrDie(mCO2Cluster.IsConstructed());
    return mCO2Cluster.Cluster();
}

} // namespace app
} // namespace chip
