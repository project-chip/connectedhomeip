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
#include <clusters/Chime/Ids.h>
#include <device/types/doorbell/Doorbell.h>
#include <devices/Types.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

namespace {
const ClusterId kClientClusters[] = { Chime::Id };

// This device should support MomentarySwitch and has 2 positions.
constexpr BitMask<Clusters::Switch::Feature> kSwitchFeatures(Clusters::Switch::Feature::kMomentarySwitch);
constexpr uint8_t kNumberOfSwitchPositions = 2;
} // namespace

Doorbell::Doorbell(const Config & config) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kDoorbell, 1)), mConfig(config)
{}

CHIP_ERROR Doorbell::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mConfig.timerDelegate).WithDelegate(&mConfig.identifyDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mSwitchCluster.Create(endpoint, kSwitchFeatures,
                          SwitchCluster::StartupConfiguration{
                              .numberOfPositions = kNumberOfSwitchPositions,
                          });
    ReturnErrorOnFailure(provider.AddCluster(mSwitchCluster.Registration()));

    mBindingCluster.Create(
        BindingCluster::Context{
            .bindingTable    = mConfig.bindingTable,
            .bindingManager  = mConfig.bindingManager,
            .platformManager = mConfig.platformManager,
        },
        endpoint);
    ReturnErrorOnFailure(provider.AddCluster(mBindingCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));

    transaction.Commit();
    return CHIP_NO_ERROR;
}

void Doorbell::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mBindingCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mBindingCluster.Cluster()));
        mBindingCluster.Destroy();
    }
    if (mSwitchCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mSwitchCluster.Cluster()));
        mSwitchCluster.Destroy();
    }
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

CHIP_ERROR Doorbell::ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const
{
    return out.ReferenceExisting(Span<const ClusterId>(kClientClusters));
}

Clusters::SwitchCluster & Doorbell::SwitchCluster()
{
    VerifyOrDie(mSwitchCluster.IsConstructed());
    return mSwitchCluster.Cluster();
}

Clusters::IdentifyCluster & Doorbell::IdentifyCluster()
{
    VerifyOrDie(mIdentifyCluster.IsConstructed());
    return mIdentifyCluster.Cluster();
}

Clusters::BindingCluster & Doorbell::BindingCluster()
{
    VerifyOrDie(mBindingCluster.IsConstructed());
    return mBindingCluster.Cluster();
}

CHIP_ERROR Doorbell::HandleShortPress()
{
    VerifyOrReturnError(mSwitchCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    // A basic doorbell short press simulates pressing the momentary switch (position 1),
    // triggering the chime, and then returning it to its idle released state (position 0).
    ChipLogProgress(AppServer, "Doorbell: Short press on endpoint %u", mEndpointId);
    ReturnErrorOnFailure(mSwitchCluster.Cluster().SetCurrentPosition(1));
    if (mSwitchCluster.Cluster().OnInitialPress(1) == std::nullopt)
    {
        ChipLogError(AppServer, "Doorbell: Unable to send OnInitialPress event");
    }
    // Chime should be triggered here.
    return mSwitchCluster.Cluster().SetCurrentPosition(0);
}

} // namespace app
} // namespace chip
