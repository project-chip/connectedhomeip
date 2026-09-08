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

#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <memory>

namespace chip {
namespace app {

// Shared audio manager for POSIX platforms. It encapsulates the miniaudio engine
// and synthesizes/manages sound resources. It provides a simple platform-agnostic
// interface for volume control and sound playback.
class PosixAudioManager
{
public:
    PosixAudioManager();
    ~PosixAudioManager();

    struct Sound
    {
        uint8_t id;
        const char * name;
    };

    // Initializes the audio engine and pre-loads/synthesizes sound resources
    CHIP_ERROR Init();

    // Shuts down the engine and releases sound resources
    void Shutdown();

    // Sets the master playback volume (scaling factor from 0 to 254)
    void SetVolume(uint8_t volume);

    // Triggers the playback of a preloaded sound by its ID
    CHIP_ERROR PlaySound(uint8_t soundId);

    // Returns the list of supported preloaded sounds
    Span<const Sound> GetSupportedSounds() const;

public:
    struct Impl;

private:
    // Pointer to internal implementation (Pimpl pattern) to hide miniaudio structures
    std::unique_ptr<Impl> mImpl;
};

} // namespace app
} // namespace chip
