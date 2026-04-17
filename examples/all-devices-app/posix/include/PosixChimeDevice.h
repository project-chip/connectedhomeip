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
    PosixChimeDevice(TimerDelegate & timerDelegate, Span<const Sound> sounds);
    ~PosixChimeDevice() override;

    Protocols::InteractionModel::Status PlayChimeSound(uint8_t chimeID) override;

private:
    ma_engine mEngine;
    bool mEngineInitialized = false;

    std::vector<uint8_t> mChime0Buffer;
    std::vector<uint8_t> mChime1Buffer;
    ma_decoder mDecoder0;
    ma_decoder mDecoder1;
    ma_sound mSound0;
    ma_sound mSound1;
    bool mSoundsInitialized = false;
};

} // namespace app
} // namespace chip
