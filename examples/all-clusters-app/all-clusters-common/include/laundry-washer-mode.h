/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <app/util/af.h>
#include <app/util/config.h>
#include <cstring>
#include <utility>

namespace chip {
namespace app {
namespace Clusters {

namespace LaundryWasherMode {

const uint8_t ModeNormal   = 0;
const uint8_t ModeDelicate = 1;
const uint8_t ModeHeavy    = 2;
const uint8_t ModeWhites   = 3;

/// This is an application level delegate to handle LaundryWasherMode commands according to the specific business logic.
class LaundryWasherModeInstance : public ModeBase::Instance
{
private:
    detail::Structs::ModeTagStruct::Type modeTagsNormal[1]   = { { .value = static_cast<uint16_t>(Clusters::LaundryWasherMode::ModeTag::kNormal) } };
    detail::Structs::ModeTagStruct::Type modeTagsDelicate[3] = { { .value = static_cast<uint16_t>(Clusters::LaundryWasherMode::ModeTag::kDelicate) },
                                              { .value = static_cast<uint16_t>(Clusters::ModeBase::ModeTag::kNight) },
                                              { .value = static_cast<uint16_t>(Clusters::ModeBase::ModeTag::kQuiet) } };
    detail::Structs::ModeTagStruct::Type modeTagsHeavy[2]    = { { .value = static_cast<uint16_t>(Clusters::ModeBase::ModeTag::kMax) },
                                           { .value = static_cast<uint16_t>(Clusters::LaundryWasherMode::ModeTag::kHeavy) } };
    detail::Structs::ModeTagStruct::Type modeTagsWhites[1]   = { { .value = static_cast<uint16_t>(Clusters::LaundryWasherMode::ModeTag::kWhites) } };

    const detail::Structs::ModeOptionStruct::Type kModeOptions[4] = {
        BuildModeOptionStruct("Normal", Clusters::LaundryWasherMode::ModeNormal, DataModel::List<const detail::Structs::ModeTagStruct::Type>(modeTagsNormal)),
        BuildModeOptionStruct("Delicate", Clusters::LaundryWasherMode::ModeDelicate,
                              DataModel::List<const detail::Structs::ModeTagStruct::Type>(modeTagsDelicate)),
        BuildModeOptionStruct("Heavy", Clusters::LaundryWasherMode::ModeHeavy, DataModel::List<const detail::Structs::ModeTagStruct::Type>(modeTagsHeavy)),
        BuildModeOptionStruct("Whites", Clusters::LaundryWasherMode::ModeWhites, DataModel::List<const detail::Structs::ModeTagStruct::Type>(modeTagsWhites)),
    };

    CHIP_ERROR AppInit() override;
    void HandleChangeToMode(uint8_t mode, ModeBase::Commands::ChangeToModeResponse::Type & response) override;

    uint8_t NumberOfModes() override { return ArraySize(kModeOptions); };
    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<detail::Structs::ModeTagStruct::Type> & tags) override;

public:
    LaundryWasherModeInstance(EndpointId aEndpointId, ClusterId aClusterId, uint32_t aFeature) :
        Instance(aEndpointId, aClusterId, aFeature){};

    ~LaundryWasherModeInstance() override = default;
};

} // namespace LaundryWasherMode

} // namespace Clusters
} // namespace app
} // namespace chip
