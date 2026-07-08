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
#include <PosixAudioManager.h>
#include <PosixChimeDevice.h>
#include <lib/support/logging/CHIPLogging.h>

#include <vector>

namespace chip {
namespace app {

namespace {
Span<const ChimeDevice::Sound> MapSounds(PosixAudioManager & audioManager)
{
    static std::vector<ChimeDevice::Sound> sMappedSounds;
    if (sMappedSounds.empty())
    {
        for (const auto & sound : audioManager.GetSupportedSounds())
        {
            sMappedSounds.push_back(ChimeDevice::Sound{ sound.id, chip::Span<const char>(sound.name, strlen(sound.name)) });
        }
    }
    return Span<const ChimeDevice::Sound>(sMappedSounds.data(), sMappedSounds.size());
}
} // namespace

PosixChimeDevice::PosixChimeDevice(TimerDelegate & timerDelegate, PosixAudioManager & audioManager) :
    ChimeDevice(timerDelegate, MapSounds(audioManager)), mAudioManager(audioManager)
{
    CHIP_ERROR err = mAudioManager.Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "PosixChimeDevice: Failed to initialize audio manager: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

Protocols::InteractionModel::Status PosixChimeDevice::PlayChimeSound(uint8_t chimeID)
{
    // Call base class to log the default message
    auto status = ChimeDevice::PlayChimeSound(chimeID);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        return status;
    }

    ChipLogProgress(DeviceLayer, "PosixChimeDevice: Delegating PlayChimeSound(%d) to PosixAudioManager", chimeID);

    CHIP_ERROR err = mAudioManager.PlaySound(chimeID);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "PosixChimeDevice: Failed to play sound %d: %" CHIP_ERROR_FORMAT, chimeID, err.Format());
        return Protocols::InteractionModel::Status::Failure;
    }

    return Protocols::InteractionModel::Status::Success;
}

} // namespace app
} // namespace chip
