/*
 *
 *    Copyright (c) 2025 Matter Authors
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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/chime-server/chime-server.h>
#include <chime-instance.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Chime;
using namespace chip::app::Clusters::Chime::Attributes;
using chip::Protocols::InteractionModel::Status;
using ChimeSoundStructType = Structs::ChimeSoundStruct::Type;

ChimeCommandDelegate ChimeCommandDelegate::instance;

ChimeSoundStructType ChimeCommandDelegate::supportedChimes[] = {
    { .chimeID = 5, .name = chip::CharSpan("Chime 5"_span) },
    { .chimeID = 10, .name = chip::CharSpan("Chime 10"_span) },
};

CHIP_ERROR ChimeCommandDelegate::GetChimeSoundByIndex(uint8_t chimeIndex, uint8_t & chimeID, MutableCharSpan & name)
{
    if (chimeIndex >= MATTER_ARRAY_SIZE(supportedChimes))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    auto & selectedChime = supportedChimes[chimeIndex];
    chimeID              = selectedChime.chimeID;
    return chip::CopyCharSpanToMutableCharSpan(selectedChime.name, name);
}

CHIP_ERROR ChimeCommandDelegate::GetChimeIDByIndex(uint8_t chimeIndex, uint8_t & chimeID)
{
    if (chimeIndex >= MATTER_ARRAY_SIZE(supportedChimes))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    auto & selectedChime = supportedChimes[chimeIndex];
    chimeID              = selectedChime.chimeID;
    return CHIP_NO_ERROR;
}

Status ChimeCommandDelegate::PlayChimeSound()
{
    return Status::Success;
}

static ChimeServer gChimeClusterServerInstance = ChimeServer(EndpointId(1), ChimeCommandDelegate::getInstance());

void emberAfChimeClusterInitCallback(EndpointId endpoint)
{
    gChimeClusterServerInstance.Init();
}
