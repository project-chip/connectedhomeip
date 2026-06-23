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

namespace {
// Global controller shared across all ProximityRangerDevice instances.
ProximityRanging::RangingTechnologyController & GetRangingController()
{
    static ProximityRanging::RangingTechnologyController sRangingController;
    return sRangingController;
}
} // namespace

namespace chip {
namespace app {

size_t ProximityRangerDevice::sActiveCount = 0;

ProximityRangerDevice::ProximityRangerDevice(TimerDelegate & timerDelegate,
                                             Span<ProximityRanging::RangingAdapter * const> adapters) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kProximityRanger, 1)),
    mRangingDriver(GetRangingController()), mTimerDelegate(timerDelegate), mAdapters(adapters)
{}

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

    // Configure cluster features based on registered adapters' technologies.
    BitMask<ProximityRanging::Feature> features;
    for (auto * adapter : mAdapters)
    {
        switch (adapter->GetTechnology())
        {
        case ProximityRanging::RangingTechEnum::kBluetoothChannelSounding:
            features.Set(ProximityRanging::Feature::kBluetoothChannelSounding);
            break;
        case ProximityRanging::RangingTechEnum::kWiFiRoundTripTimeRanging:
        case ProximityRanging::RangingTechEnum::kWiFiNextGenerationRanging:
            features.Set(ProximityRanging::Feature::kWiFiUsdProximityDetection);
            break;
        case ProximityRanging::RangingTechEnum::kBLEBeaconRSSIRanging:
            features.Set(ProximityRanging::Feature::kBleBeaconRssi);
            break;
        default:
            break;
        }
    }

    mProximityRangingCluster.Create(endpoint, ProximityRanging::ProximityRangingCluster::Config().WithFeatures(features));
    mProximityRangingCluster.Cluster().SetDriver(&mRangingDriver);
    ReturnErrorOnFailure(provider.AddCluster(mProximityRangingCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    mRegistered = true;
    sActiveCount++;
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

    if (!mRegistered)
    {
        return;
    }
    mRegistered = false;

    if (--sActiveCount != 0)
    {
        return;
    }

    for (auto * adapter : mAdapters)
    {
        if (adapter != nullptr)
        {
            LogErrorOnFailure(GetRangingController().UnregisterAdapter(*adapter));
        }
    }
}

} // namespace app
} // namespace chip
