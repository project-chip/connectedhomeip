/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "chef-chime-delegate.h"
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Chime;

namespace chip {
namespace app {
namespace Clusters {
namespace Chime {

CHIP_ERROR ChefChimeDelegate::GetChimeSoundByIndex(uint8_t chimeIndex, uint8_t & chimeID, MutableCharSpan & name)
{
    if (chimeIndex >= mChimeSounds.size())
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    chimeID = mChimeSounds[chimeIndex].id;
    return CopyCharSpanToMutableCharSpan(CharSpan::fromCharString(mChimeSounds[chimeIndex].name), name);
}

CHIP_ERROR ChefChimeDelegate::GetChimeIDByIndex(uint8_t chimeIndex, uint8_t & chimeID)
{
    if (chimeIndex >= mChimeSounds.size())
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    chimeID = mChimeSounds[chimeIndex].id;
    return CHIP_NO_ERROR;
}

Protocols::InteractionModel::Status ChefChimeDelegate::PlayChimeSound(uint8_t chimeID)
{
    if (mIsPlaying)
    {
        ChipLogProgress(Zcl, "ChefChimeDelegate: Already playing a chime sound. Ignoring request.");
        return Protocols::InteractionModel::Status::Success;
    }

    mIsPlaying = true;
    ChipLogProgress(Zcl, "ChefChimeDelegate: Playing chime sound ID %u", chimeID);

    // Simulate sound playing for 2 seconds
    (void) DeviceLayer::SystemLayer().StartTimer(
        System::Clock::Seconds16(2),
        [](System::Layer * layer, void * appState) {
            auto * delegate      = static_cast<ChefChimeDelegate *>(appState);
            delegate->mIsPlaying = false;
            ChipLogProgress(Zcl, "ChefChimeDelegate: Finished playing chime sound.");
        },
        this);

    return Protocols::InteractionModel::Status::Success;
}

} // namespace Chime
} // namespace Clusters
} // namespace app
} // namespace chip
