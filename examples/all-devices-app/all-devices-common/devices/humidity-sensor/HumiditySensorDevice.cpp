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
#include <devices/humidity-sensor/HumiditySensorDevice.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

HumiditySensorDevice::HumiditySensorDevice(TimerDelegate & timerDelegate,
                                           RelativeHumidityMeasurementCluster::Config humidityConfig) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kHumiditySensor, 1)),
    mTimerDelegate(timerDelegate), mHumidityConfig(humidityConfig)
{}

CHIP_ERROR HumiditySensorDevice::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                          EndpointComposition composition)
{
    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mRelativeHumidityMeasurementCluster.Create(endpoint, mHumidityConfig);
    ReturnErrorOnFailure(provider.AddCluster(mRelativeHumidityMeasurementCluster.Registration()));

    return provider.AddEndpoint(mEndpointRegistration);
}

void HumiditySensorDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mRelativeHumidityMeasurementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mRelativeHumidityMeasurementCluster.Cluster()));
        mRelativeHumidityMeasurementCluster.Destroy();
    }
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

} // namespace app
} // namespace chip
