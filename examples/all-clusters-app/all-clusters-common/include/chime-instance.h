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
#include <app/util/config.h>
#include <cstring>

namespace chip {
namespace app {
namespace Clusters {

/**
 * The application delegate to define the options & implement commands.
 */
class ChimeCommandDelegate : public ChimeDelegate
{
    static ChimeCommandDelegate instance;
    static chip::app::Clusters::Chime::Structs::ChimeSoundStruct::Type supportedChimes[];

public:
    CHIP_ERROR GetChimeSoundByIndex(uint8_t chimeIndex, uint8_t & chimeID, MutableCharSpan & name);

    CHIP_ERROR GetChimeIDByIndex(uint8_t chimeIndex, uint8_t & chimeID);

    Protocols::InteractionModel::Status PlayChimeSound();

    ChimeCommandDelegate()  = default;
    ~ChimeCommandDelegate() = default;

    static inline ChimeCommandDelegate & getInstance() { return instance; }
};

} // namespace Clusters
} // namespace app
} // namespace chip
