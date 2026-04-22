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

#include <devices/chime/ChimeDevice.h>
#include <memory>
#include <miniaudio.h>
#include <vector>

namespace chip {
namespace app {

// Platform-specific implementation of ChimeDevice for POSIX systems.
// It uses miniaudio for audio playback and generates sounds on-the-fly (incremental synthesis).
class PosixChimeDevice : public ChimeDevice
{
public:
    // Custom data source for miniaudio to generate sound on-the-fly.
    // This avoids loading large audio files or holding full PCM buffers in memory.
    struct CustomDataSource
    {
        ma_data_source_base base; // Base structure required by miniaudio
        double freq1;             // Primary frequency or first tone
        double freq2;             // Secondary frequency or second tone
        double duration;          // Total duration of the sound in seconds
        bool pulse;               // Whether to apply a pulse modulation effect
        ma_uint64 cursor;         // Current playback position in samples
    };

    // RAII wrapper for sound resources. Manages the lifecycle of miniaudio structures.
    class SoundResource
    {
    public:
        SoundResource(ma_engine * engine, const ChimeDevice::Sound & soundInfo);
        ~SoundResource();

        uint8_t id;                  // Chime ID matching the Matter spec
        CustomDataSource dataSource; // Custom data source for this sound
        ma_sound sound;              // Miniaudio sound object
        bool mInitialized = false;   // Whether the resource was successfully initialized
    };

    PosixChimeDevice(TimerDelegate & timerDelegate, Span<const Sound> sounds);
    ~PosixChimeDevice() override;

    Protocols::InteractionModel::Status PlayChimeSound(uint8_t chimeID) override;

private:
    ma_engine mEngine;
    bool mEngineInitialized = false;

    std::vector<std::unique_ptr<SoundResource>> mSoundResources;
    bool mSoundsInitialized = false;
};

} // namespace app
} // namespace chip
