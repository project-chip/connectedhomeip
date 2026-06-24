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
#include <lib/support/CodeUtils.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

BitMask<ProximityRanging::Feature> ProximityRangerDevice::DeriveFeatures() const
{
    BitMask<ProximityRanging::Feature> features;
    for (auto * adapter : mAdapters)
    {
        VerifyOrDie(adapter != nullptr);
        switch (adapter->GetTechnology())
        {
        case ProximityRanging::RangingTechEnum::kBLEBeaconRSSIRanging:
            features.Set(ProximityRanging::Feature::kBleBeaconRssi);
            break;
        case ProximityRanging::RangingTechEnum::kWiFiRoundTripTimeRanging:
        case ProximityRanging::RangingTechEnum::kWiFiNextGenerationRanging:
            features.Set(ProximityRanging::Feature::kWiFiUsdProximityDetection);
            break;
        case ProximityRanging::RangingTechEnum::kBluetoothChannelSounding:
            features.Set(ProximityRanging::Feature::kBluetoothChannelSounding);
            break;
        default:
            break;
        }
    }
    return features;
}

ProximityRangerDevice::ProximityRangerDevice(TimerDelegate & timerDelegate,
                                             std::vector<Clusters::ProximityRanging::RangingAdapter *> adapters) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kProximityRanger, 1)),
    mTimerDelegate(timerDelegate), mAdapters(std::move(adapters))
{}

CHIP_ERROR ProximityRangerDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                           EndpointComposition composition)
{
    VerifyOrReturnError(!mRegistered, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    CHIP_ERROR err = provider.AddCluster(mIdentifyCluster.Registration());
    if (err != CHIP_NO_ERROR)
    {
        // AddCluster failed, so the cluster was never registered with the provider; destroy
        // the constructed object directly rather than going through Unregister (which would
        // call provider.RemoveCluster on a cluster the provider never accepted).
        mIdentifyCluster.Destroy();
        Unregister(provider);
        return err;
    }

    mProximityRangingCluster.Create(
        endpoint,
        ProximityRanging::ProximityRangingCluster::Config(mTimerDelegate)
            .WithFeatures(DeriveFeatures())
            .WithAdapters(Span<ProximityRanging::RangingAdapter * const>(mAdapters.data(), mAdapters.size())));
    err = provider.AddCluster(mProximityRangingCluster.Registration());
    if (err != CHIP_NO_ERROR)
    {
        // See above: AddCluster failed, so destroy this cluster directly. Unregister still
        // unwinds the previously-registered mIdentifyCluster.
        mProximityRangingCluster.Destroy();
        Unregister(provider);
        return err;
    }

    err = provider.AddEndpoint(mEndpointRegistration);
    if (err != CHIP_NO_ERROR)
    {
        Unregister(provider);
        return err;
    }

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
