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

Doorbell::Doorbell(TimerDelegate & timerDelegate, Span<const Sound> sounds) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kDoorbell, 1)), mTimerDelegate(timerDelegate), mSounds(sounds)
{}

CHIP_ERROR Doorbell::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    SwitchCluster::StartupConfiguration switchConfig = {2, 2};
    mSwitchCluster.Create(endpoint, BitFlags<Switch::Feature>(Switch::Feature::kMomentarySwitch), switchConfig);
    ReturnErrorOnFailure(provider.AddCluster(mSwitchCluster.Registration()));

    mChimeCluster.Create(endpoint, *this);
    ReturnErrorOnFailure(provider.AddCluster(mChimeCluster.Registration()));

    return provider.AddEndpoint(mEndpointRegistration);
}

void Doorbell::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mChimeCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mChimeCluster.Cluster()));
        mChimeCluster.Destroy();
    }
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

Clusters::ChimeCluster & Doorbell::ChimeCluster()
{
    VerifyOrDie(mChimeCluster.IsConstructed());
    return mChimeCluster.Cluster();
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

CHIP_ERROR Doorbell::GetChimeSoundByIndex(uint8_t chimeIndex, uint8_t & chimeID, MutableCharSpan & name)
{
    if (chimeIndex >= mSounds.size())
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    const auto & sound = mSounds[chimeIndex];
    chimeID            = sound.id;
    return CopyCharSpanToMutableCharSpan(sound.name, name);
}

CHIP_ERROR Doorbell::GetChimeIDByIndex(uint8_t chimeIndex, uint8_t & chimeID)
{
    if (chimeIndex >= mSounds.size())
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    chimeID = mSounds[chimeIndex].id;
    return CHIP_NO_ERROR;
}

Protocols::InteractionModel::Status Doorbell::PlayChimeSound(uint8_t chimeID)
{
    CharSpan soundName = "Unknown"_span;
    for (const auto & sound : mSounds)
    {
        if (sound.id == chimeID)
        {
            soundName = sound.name;
            break;
        }
    }

    ChipLogProgress(AppServer, "Doorbell: Playing sound %s", chip::NullTerminated(soundName).c_str());

    return Protocols::InteractionModel::Status::Success;
}

} // namespace app
} // namespace chip
