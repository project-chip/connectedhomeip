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
#include <devices/Types.h>
#include <devices/chime/ChimeDevice.h>
#include <lib/support/StringBuilder.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

ChimeDevice::ChimeDevice(TimerDelegate & timerDelegate, Span<const Sound> sounds) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kChime, 1)), mTimerDelegate(timerDelegate),
    mSounds(sounds)
{}

CHIP_ERROR ChimeDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    ReturnErrorOnFailure(SingleEndpointRegistration(endpoint, provider, parentId));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mChimeCluster.Create(endpoint, *this);
    ReturnErrorOnFailure(provider.AddCluster(mChimeCluster.Registration()));

    return provider.AddEndpoint(mEndpointRegistration);
}

void ChimeDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    SingleEndpointUnregistration(provider);
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

Clusters::ChimeCluster & ChimeDevice::ChimeCluster()
{
    VerifyOrDie(mChimeCluster.IsConstructed());
    return mChimeCluster.Cluster();
}

CHIP_ERROR ChimeDevice::GetChimeSoundByIndex(uint8_t chimeIndex, uint8_t & chimeID, MutableCharSpan & name)
{
    if (chimeIndex >= mSounds.size())
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    const auto & sound = mSounds[chimeIndex];
    chimeID            = sound.id;
    return CopyCharSpanToMutableCharSpan(sound.name, name);
}

CHIP_ERROR ChimeDevice::GetChimeIDByIndex(uint8_t chimeIndex, uint8_t & chimeID)
{
    if (chimeIndex >= mSounds.size())
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    chimeID = mSounds[chimeIndex].id;
    return CHIP_NO_ERROR;
}

Protocols::InteractionModel::Status ChimeDevice::PlayChimeSound(uint8_t chimeID)
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

    ChipLogProgress(AppServer, "ChimeDevice: Playing sound %s", chip::NullTerminated(soundName).c_str());

    return Protocols::InteractionModel::Status::Success;
}

} // namespace app
} // namespace chip
