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
#pragma once

#include <devices/speaker/SpeakerDevice.h>

#include <PosixAudioManager.h>

namespace chip {
namespace app {

// Platform-specific implementation of SpeakerDevice for POSIX systems.
// It overrides LevelControl and OnOff delegate methods to adjust the master
// volume of the shared miniaudio engine.
class PosixSpeakerDevice : public SpeakerDevice, public Clusters::LevelControlDelegate, public Clusters::OnOffDelegate
{
public:
    struct Context
    {
        TimerDelegate & timerDelegate;
    };

    PosixSpeakerDevice(const Context & context, PosixAudioManager & audioManager);
    ~PosixSpeakerDevice() override;

    // LevelControlDelegate
    void OnLevelChanged(uint8_t value) override;
    void OnDefaultMoveRateChanged(DataModel::Nullable<uint8_t> value) override {}
    void OnOptionsChanged(BitMask<Clusters::LevelControl::OptionsBitmap> value) override {}
    void OnOnLevelChanged(DataModel::Nullable<uint8_t> value) override {}

    // OnOffDelegate
    void OnOffStartup(bool on) override;
    void OnOnOffChanged(bool on) override;

private:
    PosixAudioManager & mAudioManager;
    void UpdateEngineVolume();
};

} // namespace app
} // namespace chip
