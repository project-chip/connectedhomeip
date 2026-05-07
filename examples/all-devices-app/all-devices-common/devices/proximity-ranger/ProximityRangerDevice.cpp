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
ProximityRanging::RangingTechnologyController sRangingController;
} // namespace

namespace chip {
namespace app {

namespace {

BitMask<ProximityRanging::Feature> FeatureMapFromAdapters(Span<ProximityRanging::RangingAdapter * const> adapters)
{
    BitMask<ProximityRanging::Feature> features;
    for (auto * adapter : adapters)
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
    return features;
}

} // namespace

ProximityRangerDevice::ProximityRangerDevice(TimerDelegate & timerDelegate,
                                             Span<ProximityRanging::RangingAdapter * const> adapters) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kProximityRanger, 1)),
    mRangingDriver(sRangingController, FeatureMapFromAdapters(adapters)), mTimerDelegate(timerDelegate), mAdapters(adapters)
{}

CHIP_ERROR ProximityRangerDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    // Register adapters with the shared controller, skipping any already registered.
    for (auto * adapter : mAdapters)
    {
        VerifyOrReturnError(adapter != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        CHIP_ERROR err = sRangingController.RegisterAdapter(*adapter);
        if (err != CHIP_NO_ERROR && err != CHIP_ERROR_DUPLICATE_KEY_ID)
        {
            return err;
        }
    }

    ReturnErrorOnFailure(SingleEndpointRegistration(endpoint, provider, parentId));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    // Compute optional attributes from driver feature map.
    auto features = mRangingDriver.GetFeatureMap();
    ProximityRanging::ProximityRangingCluster::OptionalAttributes optionalAttrs;
    if (features.Has(ProximityRanging::Feature::kWiFiUsdProximityDetection))
    {
        optionalAttrs.Set<ProximityRanging::Attributes::WiFiDevIK::Id>();
    }
    if (features.Has(ProximityRanging::Feature::kBleBeaconRssi))
    {
        optionalAttrs.Set<ProximityRanging::Attributes::BLEDeviceID::Id>();
    }
    if (features.Has(ProximityRanging::Feature::kBluetoothChannelSounding))
    {
        optionalAttrs.Set<ProximityRanging::Attributes::BLTDevIK::Id>();
        optionalAttrs.Set<ProximityRanging::Attributes::BLTCSSecurityLevel::Id>();
        optionalAttrs.Set<ProximityRanging::Attributes::BLTCSModeCapability::Id>();
    }

    mProximityRangingCluster.Create(endpoint, optionalAttrs);
    mProximityRangingCluster.Cluster().SetDriver(&mRangingDriver);
    ReturnErrorOnFailure(provider.AddCluster(mProximityRangingCluster.Registration()));

    return provider.AddEndpoint(mEndpointRegistration);
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

    // Unregister adapters from the shared controller.
    for (auto * adapter : mAdapters)
    {
        if (adapter != nullptr)
        {
            LogErrorOnFailure(sRangingController.UnregisterAdapter(*adapter));
        }
    }
}

} // namespace app
} // namespace chip
