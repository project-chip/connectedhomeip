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
#include <PosixSpeakerDevice.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {

PosixSpeakerDevice::PosixSpeakerDevice(const Context & context, PosixAudioManager & audioManager) :
    SpeakerDevice(*this, *this, context.timerDelegate), mAudioManager(audioManager)
{
    CHIP_ERROR err = mAudioManager.Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "PosixSpeakerDevice: Failed to initialize audio manager: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

PosixSpeakerDevice::~PosixSpeakerDevice() {}

void PosixSpeakerDevice::OnLevelChanged(uint8_t value)
{
    UpdateEngineVolume();
}

void PosixSpeakerDevice::OnOffStartup(bool on)
{
    UpdateEngineVolume();
}

void PosixSpeakerDevice::OnOnOffChanged(bool on)
{
    UpdateEngineVolume();
}

void PosixSpeakerDevice::UpdateEngineVolume()
{
    bool muted    = !OnOffCluster().GetOnOff();
    uint8_t level = LevelControlCluster().GetCurrentLevel().ValueOr(Clusters::LevelControlCluster::kMaxLevel);

    uint8_t volume = !muted ? level : 0;
    mAudioManager.SetVolume(volume);

    ChipLogProgress(DeviceLayer, "PosixSpeakerDevice: Updated master volume to %u (Muted: %d, Level: %u)", volume, muted, level);
}

} // namespace app
} // namespace chip
