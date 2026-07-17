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
#include <device/types/ambient-context-sensor/AmbientContextSensor.h>
#include <devices/Types.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip::app {

AmbientContextSensor::AmbientContextSensor(AmbientContextSensingConfig config, TimerDelegate & timerDelegate,
                                           AmbientContextSensing::AmbientContextSensingDelegate & delegate) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kAmbientContextSensor, 1)),
    mConfig(config), mTimerDelegate(timerDelegate), mDelegate(delegate)
{}

CHIP_ERROR AmbientContextSensor::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                          EndpointComposition composition)
{
    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    // Create the identify cluster.
    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    // Create the ambient context sensing cluster
    mAmbientContextSensingCluster.Create(endpoint, mConfig);
    AmbientContextSensingCluster().SetDelegate(&mDelegate);
    ReturnErrorOnFailure(provider.AddCluster(mAmbientContextSensingCluster.Registration()));

    return provider.AddEndpoint(mEndpointRegistration);
}

void AmbientContextSensor::Unregister(CodeDrivenDataModelProvider & provider)
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
