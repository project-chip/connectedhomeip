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
#include "ChimeDeviceImpl.h"
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

ChimeDeviceImpl::ChimeDeviceImpl() :
    ChimeDevice(*this, &mTimerDelegate)
{
    mSounds.push_back({0, "Ding Dong"});
    mSounds.push_back({1, "Ring Ring"});
}

CHIP_ERROR ChimeDeviceImpl::GetChimeSoundByIndex(uint8_t chimeIndex, uint8_t & chimeID, MutableCharSpan & name)
{
    if (chimeIndex >= mSounds.size())
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    
    const auto & sound = mSounds[chimeIndex];
    chimeID = sound.id;
    return CopyCharSpanToMutableCharSpan(CharSpan::fromCharString(sound.name.c_str()), name);
}

CHIP_ERROR ChimeDeviceImpl::GetChimeIDByIndex(uint8_t chimeIndex, uint8_t & chimeID)
{
    if (chimeIndex >= mSounds.size())
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    
    chimeID = mSounds[chimeIndex].id;
    return CHIP_NO_ERROR;
}

Protocols::InteractionModel::Status ChimeDeviceImpl::PlayChimeSound()
{
    // Access the cluster to get the selected chime attribute
    // Note: ChimeDevice exposes ChimeCluster() accessor
    uint8_t selectedChime = ChimeCluster().GetSelectedChime();
    
    // Find the name
    const char * soundName = "Unknown";
    for (const auto & sound : mSounds)
    {
        if (sound.id == selectedChime)
        {
            soundName = sound.name.c_str();
            break;
        }
    }

    ChipLogProgress(AppServer, "ChimeDeviceImpl: Playing sound %s", soundName);
    return Protocols::InteractionModel::Status::Success;
}
