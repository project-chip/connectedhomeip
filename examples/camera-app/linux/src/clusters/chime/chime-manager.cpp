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

#include "chime-manager.h"
#include <iostream>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Chime;
using namespace chip::app::Clusters::Chime::Structs;

using namespace Camera;

// Chime Cluster Methods
CHIP_ERROR ChimeManager::GetChimeSoundByIndex(uint8_t chimeIndex, uint8_t & chimeID, MutableCharSpan & name)
{
    if (chimeIndex >= MATTER_ARRAY_SIZE(mChimeSounds))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    auto & selectedChime = mChimeSounds[chimeIndex];
    chimeID              = selectedChime.chimeID;
    return chip::CopyCharSpanToMutableCharSpan(selectedChime.name, name);
}

CHIP_ERROR ChimeManager::GetChimeIDByIndex(uint8_t chimeIndex, uint8_t & chimeID)
{
    if (chimeIndex >= MATTER_ARRAY_SIZE(mChimeSounds))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    auto & selectedChime = mChimeSounds[chimeIndex];
    chimeID              = selectedChime.chimeID;
    return CHIP_NO_ERROR;
}

Protocols::InteractionModel::Status ChimeManager::PlayChimeSound()
{
    // check if we are enabled
    if (!mChimeServer->GetEnabled())
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    // Get the Active Chime ID
    auto selectedChime = mChimeServer->GetSelectedChime();

    // Play chime sound
    ChipLogDetail(Zcl, "Playing Chime with sound ID: %u", unsigned(selectedChime));
    return Protocols::InteractionModel::Status::Success;
}
