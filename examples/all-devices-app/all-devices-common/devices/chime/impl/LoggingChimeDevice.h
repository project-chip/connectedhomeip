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
#pragma once

#include <app/clusters/chime-server/ChimeCluster.h>
#include <devices/chime/ChimeDevice.h>
#include <lib/support/Span.h>
#include <platform/DefaultTimerDelegate.h>
#include <vector>

namespace chip {
namespace app {

/**
 * @brief An implementation of a Chime Device.
 *
 * This class serves as a simple example of a chime. It implements the ChimeDelegate
 * interface and logs a message ("Ding Dong" or "Ring Ring") when a chime sound is played.
 */
class LoggingChimeDevice : public ChimeDevice, public Clusters::ChimeDelegate
{
public:
    LoggingChimeDevice();
    ~LoggingChimeDevice() override = default;

    // ChimeDelegate
    CHIP_ERROR GetChimeSoundByIndex(uint8_t chimeIndex, uint8_t & chimeID, MutableCharSpan & name) override;
    CHIP_ERROR GetChimeIDByIndex(uint8_t chimeIndex, uint8_t & chimeID) override;
    Protocols::InteractionModel::Status PlayChimeSound(uint8_t chimeID) override;

private:
    DefaultTimerDelegate mTimerDelegate;

    struct ChimeSound
    {
        uint8_t id;
        CharSpan name;
    };
    std::vector<ChimeSound> mSounds;
};

} // namespace app
} // namespace chip
