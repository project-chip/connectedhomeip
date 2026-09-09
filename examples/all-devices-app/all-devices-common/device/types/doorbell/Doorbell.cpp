/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <device/types/doorbell/Doorbell.h>
#include <devices/Types.h>
#include <lib/support/StringBuilder.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

namespace {
    const ClusterId kClientClusters[] = { Chime::Id };
    } // namespace

Doorbell::Doorbell(TimerDelegate & timerDelegate) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kDoorbell, 1)), mTimerDelegate(timerDelegate)
{}

CHIP_ERROR Doorbell::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    SwitchCluster::StartupConfiguration switchConfig = {2, 2};
    mSwitchCluster.Create(endpoint, BitFlags<Switch::Feature>(Switch::Feature::kMomentarySwitch), switchConfig);
    ReturnErrorOnFailure(provider.AddCluster(mSwitchCluster.Registration()));

    return provider.AddEndpoint(mEndpointRegistration);
}

void Doorbell::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
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

} // namespace app
} // namespace chip
