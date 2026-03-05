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

namespace chip {
namespace app {

/**
 * @brief An implementation of a Speaker Device.
 *
 * This class serves as a simple example. It implements the LevelControlDelegate and OnOffDelegate
 * interfaces and logs messages when speaker actions occur (Mute/Unmute, Volume Change).
 * It also handles the interaction between OnOff (Mute) and LevelControl (Volume) clusters.
 */
class LoggingSpeakerDevice : public SpeakerDevice, public Clusters::LevelControlDelegate, public Clusters::OnOffDelegate
{
public:
    struct Context
    {
        TimerDelegate & timerDelegate;
    };

    LoggingSpeakerDevice(const Context & context);
    ~LoggingSpeakerDevice() override;

    // LevelControlDelegate
    void OnLevelChanged(uint8_t value) override;
    void OnDefaultMoveRateChanged(DataModel::Nullable<uint8_t> value) override;
    void OnOptionsChanged(BitMask<Clusters::LevelControl::OptionsBitmap> value) override;
    void OnOnLevelChanged(DataModel::Nullable<uint8_t> value) override;

    // OnOffDelegate
    void OnOffStartup(bool on) override;
    void OnOnOffChanged(bool on) override;
};

} // namespace app
} // namespace chip
