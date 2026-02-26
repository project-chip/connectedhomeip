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
#include <devices/chime/impl/LoggingChimeDevice.h>
#include <lib/support/StringBuilder.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

LoggingChimeDevice::LoggingChimeDevice() : ChimeDevice(*this, mTimerDelegate)
{
    mSounds.push_back({ 0, "Ding Dong"_span });
    mSounds.push_back({ 1, "Ring Ring"_span });
}

CHIP_ERROR LoggingChimeDevice::GetChimeSoundByIndex(uint8_t chimeIndex, uint8_t & chimeID, MutableCharSpan & name)
{
    if (chimeIndex >= mSounds.size())
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    const auto & sound = mSounds[chimeIndex];
    chimeID            = sound.id;
    return CopyCharSpanToMutableCharSpan(sound.name, name);
}

CHIP_ERROR LoggingChimeDevice::GetChimeIDByIndex(uint8_t chimeIndex, uint8_t & chimeID)
{
    if (chimeIndex >= mSounds.size())
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    chimeID = mSounds[chimeIndex].id;
    return CHIP_NO_ERROR;
}

Protocols::InteractionModel::Status LoggingChimeDevice::PlayChimeSound(uint8_t chimeID)
{
    // The chime ID to be played is provided to the app

    // Find the name
    CharSpan soundName = "Unknown"_span;
    for (const auto & sound : mSounds)
    {
        if (sound.id == chimeID)
        {
            soundName = sound.name;
            break;
        }
    }

    ChipLogProgress(AppServer, "LoggingChimeDevice: Playing sound %s", chip::NullTerminated(soundName).c_str());
    return Protocols::InteractionModel::Status::Success;
}

} // namespace app
} // namespace chip
