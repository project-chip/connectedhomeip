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

#pragma once

#include <app/clusters/chime-server/ChimeCluster.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Chime {

struct ChimeSound
{
    uint8_t id;
    const char * name;
};

static const ChimeSound kDefaultChimeSounds[] = {
    { 1, "Classic Ding Dong" },
    { 2, "Merry Melodies" },
    { 3, "Digital Alert" },
};

static constexpr size_t kNumDefaultChimeSounds = sizeof(kDefaultChimeSounds) / sizeof(kDefaultChimeSounds[0]);

class ChefChimeDelegate : public ChimeDelegate
{
public:
    ChefChimeDelegate() : mChimeSounds(kDefaultChimeSounds) {}
    ChefChimeDelegate(Span<const ChimeSound> chimeSounds) : mChimeSounds(chimeSounds) {}
    ~ChefChimeDelegate() override = default;

    CHIP_ERROR GetChimeSoundByIndex(uint8_t chimeIndex, uint8_t & chimeID, MutableCharSpan & name) override;
    CHIP_ERROR GetChimeIDByIndex(uint8_t chimeIndex, uint8_t & chimeID) override;
    Protocols::InteractionModel::Status PlayChimeSound(uint8_t chimeID) override;

private:
    Span<const ChimeSound> mChimeSounds;
    bool mIsPlaying = false;
};

} // namespace Chime
} // namespace Clusters
} // namespace app
} // namespace chip
