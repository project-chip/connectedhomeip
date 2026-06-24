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
#include <devices/ambient-context-sensor/AmbientContextSensorDevice.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip::app {

AmbientContextSensorDevice::AmbientContextSensorDevice(AmbientContextSensingConfig config, TimerDelegate & timerDelegate) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kAmbientContextSensor, 1)), mConfig(config),
    mTimerDelegate(timerDelegate)
{}

CHIP_ERROR AmbientContextSensorDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                                EndpointComposition composition)
{
    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    // Create the identify cluster.
    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    // Update the config with the actual endpoint ID
    mConfig.mEndpointId = endpoint;

    // Create the ambient context sensing cluster
    mAmbientContextSensingCluster.Create(mConfig);
    ReturnErrorOnFailure(provider.AddCluster(mAmbientContextSensingCluster.Registration()));

    return provider.AddEndpoint(mEndpointRegistration);
}

void AmbientContextSensorDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mAmbientContextSensingCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mAmbientContextSensingCluster.Cluster()));
        mAmbientContextSensingCluster.Destroy();
    }
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

} // namespace chip::app
