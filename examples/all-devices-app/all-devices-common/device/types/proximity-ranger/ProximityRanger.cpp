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
#include <device/types/proximity-ranger/ProximityRanger.h>
#include <devices/Types.h>
#include <lib/support/CodeUtils.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

BitMask<ProximityRanging::Feature> ProximityRanger::DeriveFeatures() const
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

ProximityRanger::ProximityRanger(TimerDelegate & timerDelegate,
                                 std::vector<Clusters::ProximityRanging::RangingAdapter *> adapters) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kProximityRanger, 1)),
    mTimerDelegate(timerDelegate), mAdapters(std::move(adapters))
{}

CHIP_ERROR ProximityRanger::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                     EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    VerifyOrReturnError(!mRegistered, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mProximityRangingCluster.Create(
        endpoint,
        ProximityRanging::ProximityRangingCluster::Config(mTimerDelegate)
            .WithFeatures(DeriveFeatures())
            .WithAdapters(Span<ProximityRanging::RangingAdapter * const>(mAdapters.data(), mAdapters.size())));
    ReturnErrorOnFailure(provider.AddCluster(mProximityRangingCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    mRegistered = true;
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void ProximityRanger::Unregister(CodeDrivenDataModelProvider & provider)
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
