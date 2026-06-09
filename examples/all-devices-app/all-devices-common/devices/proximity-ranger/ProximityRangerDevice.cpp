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
#include <devices/proximity-ranger/impl/LoggingRangingAdapter.h>
#include <lib/support/CodeUtils.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {
namespace {

ProximityRanging::ProximityRangingDriver * gDriver = nullptr;

// Span over the same fixed adapter set the driver was constructed with.
// Populated atomically with gDriver on first ProximityRangerDevice
// construction and read by Register() to derive the cluster's feature map
// from the adapter technologies.
Span<ProximityRanging::RangingAdapter * const> gAdapterSet;

} // namespace

ProximityRanging::ProximityRangingDriver & ProximityRangerDevice::GetRangingDriver()
{
    // The first ProximityRangerDevice constructor sets gDriver via the
    // static initialization block below. Calling this before any
    // ProximityRangerDevice exists is a programming error.
    VerifyOrDie(gDriver != nullptr);
    return *gDriver;
}

ProximityRangerDevice::ProximityRangerDevice(TimerDelegate & timerDelegate, PersistentStorageDelegate & storage) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kProximityRanger, 1)), mTimerDelegate(timerDelegate)
{
    // Function-local statics give us first-construction-wins semantics for
    // the shared driver and its adapter set: timerDelegate and storage are
    // captured on the first ProximityRangerDevice construction and ignored
    // thereafter. The driver lives until process exit, so the adapters
    // (also static) outlive it.
    static ProximityRanging::LoggingRangingAdapter sLoggingBleAdapter(ProximityRanging::RangingTechEnum::kBLEBeaconRSSIRanging,
                                                                      timerDelegate, &storage, true);
    static ProximityRanging::LoggingRangingAdapter sLoggingWiFiAdapter(ProximityRanging::RangingTechEnum::kWiFiRoundTripTimeRanging,
                                                                       timerDelegate);
    static ProximityRanging::LoggingRangingAdapter sLoggingBltcsAdapter(
        ProximityRanging::RangingTechEnum::kBluetoothChannelSounding, timerDelegate);
    static ProximityRanging::RangingAdapter * sAdapters[] = {
        &sLoggingBleAdapter,
        &sLoggingWiFiAdapter,
        &sLoggingBltcsAdapter,
    };
    static ProximityRanging::ProximityRangingDriver sDriver{ Span<ProximityRanging::RangingAdapter * const>(sAdapters),
                                                             timerDelegate };
    if (gDriver == nullptr)
    {
        gDriver     = &sDriver;
        gAdapterSet = Span<ProximityRanging::RangingAdapter * const>(sAdapters);
    }
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

    // Derive the cluster's feature map from the registered adapters'
    // technologies so the feature set is a function of what the device can
    // actually do, not a separate constant the caller has to keep in sync.
    BitMask<ProximityRanging::Feature> features;
    for (auto * adapter : gAdapterSet)
    {
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

    mProximityRangingCluster.Create(endpoint,
                                    ProximityRanging::ProximityRangingCluster::Config(GetRangingDriver()).WithFeatures(features));
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
