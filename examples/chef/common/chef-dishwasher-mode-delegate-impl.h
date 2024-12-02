/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/util/config.h>
#include <cstring>
#include <utility>

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {

namespace DishwasherMode {

const uint8_t ModeNormal = 0;
const uint8_t ModeHeavy  = 1;
const uint8_t ModeLight  = 2;

/// This is an application level delegate to handle DishwasherMode commands according to the specific business logic.
class DishwasherModeDelegate : public ModeBase::Delegate
{
private:
    using ModeTagStructType             = detail::Structs::ModeTagStruct::Type;
    ModeTagStructType modeTagsNormal[1] = { { .value = to_underlying(ModeTag::kNormal) } };
    ModeTagStructType modeTagsHeavy[2]  = { { .value = to_underlying(ModeBase::ModeTag::kMax) },
                                            { .value = to_underlying(ModeTag::kHeavy) } };
    ModeTagStructType modeTagsLight[3]  = { { .value = to_underlying(ModeTag::kLight) },
                                            { .value = to_underlying(ModeBase::ModeTag::kNight) },
                                            { .value = to_underlying(ModeBase::ModeTag::kQuiet) } };

    const detail::Structs::ModeOptionStruct::Type kModeOptions[3] = {
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Normal"),
                                                 .mode     = ModeNormal,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(modeTagsNormal) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Heavy"),
                                                 .mode     = ModeHeavy,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(modeTagsHeavy) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Light"),
                                                 .mode     = ModeLight,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(modeTagsLight) }
    };

    CHIP_ERROR Init() override;
    void HandleChangeToMode(uint8_t mode, ModeBase::Commands::ChangeToModeResponse::Type & response) override;

    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<ModeTagStructType> & tags) override;

public:
    ~DishwasherModeDelegate() override = default;
};

ModeBase::Instance * Instance();

void Shutdown();

} // namespace DishwasherMode

} // namespace Clusters
} // namespace app
} // namespace chip

#ifdef MATTER_DM_PLUGIN_DISHWASHER_MODE_SERVER
chip::Protocols::InteractionModel::Status chefDishwasherModeWriteCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                                          const EmberAfAttributeMetadata * attributeMetadata,
                                                                          uint8_t * buffer);
chip::Protocols::InteractionModel::Status chefDishwasherModeReadCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                                         const EmberAfAttributeMetadata * attributeMetadata,
                                                                         uint8_t * buffer, uint16_t maxReadLength);
#endif // MATTER_DM_PLUGIN_DISHWASHER_MODE_SERVER
