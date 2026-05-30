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

ProximityRanging::ProximityRangingDriver &
ProximityRangerDevice::GetRangingDriver(Span<ProximityRanging::RangingAdapter * const> adapters)
{
    // Meyer's singleton: initialized on the first call with the supplied
    // adapter set, subsequent calls return the same instance and ignore
    // their `adapters` argument. The DeviceFactory passes the same static
    // adapter array on every Create("proximity-ranger") so the
    // first-wins semantic is well-defined for the only production caller.
    static ProximityRanging::ProximityRangingDriver instance{ adapters };
    return instance;
}

ProximityRangerDevice::ProximityRangerDevice(TimerDelegate & timerDelegate, Span<ProximityRanging::RangingAdapter * const> adapters,
                                             BitMask<ProximityRanging::Feature> features) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kProximityRanger, 1)),
    mTimerDelegate(timerDelegate), mFeatures(features)
{
    // Trigger lazy initialization of the shared driver. On subsequent
    // device constructions this is a no-op; the same static instance
    // is reused across every ProximityRangerDevice.
    (void) GetRangingDriver(adapters);
}

CHIP_ERROR ProximityRangerDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                           EndpointComposition composition)
{
    VerifyOrReturnError(!mRegistered, CHIP_ERROR_INCORRECT_STATE);

    // Register adapters with the shared controller, skipping any already registered.
    for (auto * adapter : mAdapters)
    {
        VerifyOrReturnError(adapter != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        ReturnErrorOnFailure(GetRangingController().RegisterAdapter(*adapter).NoErrorIf(CHIP_ERROR_DUPLICATE_KEY_ID));
    }

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mProximityRangingCluster.Create(endpoint,
                                    ProximityRanging::ProximityRangingCluster::Config(GetRangingDriver()).WithFeatures(mFeatures));
    ReturnErrorOnFailure(provider.AddCluster(mProximityRangingCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    mRegistered = true;
    return CHIP_NO_ERROR;
}

void ProximityRangerDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);

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
