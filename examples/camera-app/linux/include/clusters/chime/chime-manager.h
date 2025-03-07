/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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
#include <app/clusters/chime-server/chime-server.h>

namespace Camera {

class ChimeManager : public chip::app::Clusters::ChimeDelegate
{

public:
    ChimeManager() {}

    // Chime Delegate methods
    CHIP_ERROR GetChimeSoundByIndex(uint8_t chimeIndex, uint8_t & chimeID, chip::MutableCharSpan & name);
    CHIP_ERROR GetChimeIDByIndex(uint8_t chimeIndex, uint8_t & chimeID);

    chip::Protocols::InteractionModel::Status PlayChimeSound();

private:
    using ChimeSoundStructType = chip::app::Clusters::Chime::Structs::ChimeSoundStruct::Type;

    const ChimeSoundStructType mChimeSounds[1] = {
        ChimeSoundStructType{ .chimeID = 0, .name = chip::CharSpan::fromCharString("Basic Door Chime") },
    };
};

} // namespace Camera
