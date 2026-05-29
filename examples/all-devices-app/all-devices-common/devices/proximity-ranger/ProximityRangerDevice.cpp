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
#include <devices/proximity-ranger/ProximityRangerDevice.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

ProximityRangerDevice::ProximityRangerDevice(TimerDelegate & timerDelegate, Span<ProximityRanging::RangingAdapter * const> adapters,
                                             BitMask<ProximityRanging::Feature> features) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kProximityRanger, 1)),
    mTimerDelegate(timerDelegate), mDriver(adapters), mFeatures(features)
{}

CHIP_ERROR ProximityRangerDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    VerifyOrReturnError(!mRegistered, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(SingleEndpointRegistration(endpoint, provider, parentId));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mProximityRangingCluster.Create(endpoint, ProximityRanging::ProximityRangingCluster::Config(mDriver).WithFeatures(mFeatures));
    ReturnErrorOnFailure(provider.AddCluster(mProximityRangingCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    mRegistered = true;
    return CHIP_NO_ERROR;
}

void ProximityRangerDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    SingleEndpointUnregistration(provider);

    if (mProximityRangingCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mProximityRangingCluster.Cluster()));
        mProximityRangingCluster.Destroy();
    }

    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }

    mRegistered = false;
}

} // namespace app
} // namespace chip
