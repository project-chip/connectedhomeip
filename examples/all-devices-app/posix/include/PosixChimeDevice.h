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
#include <miniaudio.h>
#include <vector>

namespace chip {
namespace app {

class PosixChimeDevice : public ChimeDevice
{
public:
    struct CustomDataSource
    {
        ma_data_source_base base;
        double freq1;
        double freq2;
        double duration;
        bool pulse;
        ma_uint64 cursor;
    };

    struct SoundResource
    {
        uint8_t id;
        CustomDataSource dataSource;
        ma_sound sound;
        bool initialized = false;
    };

    PosixChimeDevice(TimerDelegate & timerDelegate, Span<const Sound> sounds);
    ~PosixChimeDevice() override;

    Protocols::InteractionModel::Status PlayChimeSound(uint8_t chimeID) override;

private:
    ma_engine mEngine;
    bool mEngineInitialized = false;

    std::vector<SoundResource> mSoundResources;
    bool mSoundsInitialized = false;
};

} // namespace app
} // namespace chip
