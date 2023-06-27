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

template <typename T>
using List                 = chip::app::DataModel::List<T>;
using ModeTagStructType    = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;
using ModeOptionStructType = chip::app::Clusters::detail::Structs::ModeOptionStruct::Type;

namespace chip {
namespace app {
namespace Clusters {

namespace DishwasherMode {

const uint8_t ModeNormal = 0;
const uint8_t ModeHeavy  = 1;
const uint8_t ModeLight  = 2;

/// This is an application level delegate to handle DishwasherMode commands according to the specific business logic.
class DishwasherModeInstance : public ModeBase::Instance
{
private:
    ModeTagStructType modeTagsNormal[1] = { { .value = static_cast<uint16_t>(Clusters::DishwasherMode::ModeTag::kNormal) } };
    ModeTagStructType modeTagsHeavy[2]  = { { .value = static_cast<uint16_t>(Clusters::ModeBase::ModeTag::kMax) },
                                           { .value = static_cast<uint16_t>(Clusters::DishwasherMode::ModeTag::kHeavy) } };
    ModeTagStructType modeTagsLight[3]  = { { .value = static_cast<uint16_t>(Clusters::DishwasherMode::ModeTag::kLight) },
                                           { .value = static_cast<uint16_t>(Clusters::ModeBase::ModeTag::kNight) },
                                           { .value = static_cast<uint16_t>(Clusters::ModeBase::ModeTag::kQuiet) } };

    const ModeOptionStructType modeOptions[3] = {
        BuildModeOptionStruct("Normal", Clusters::DishwasherMode::ModeNormal, List<const ModeTagStructType>(modeTagsNormal)),
        BuildModeOptionStruct("Heavy", Clusters::DishwasherMode::ModeHeavy, List<const ModeTagStructType>(modeTagsHeavy)),
        BuildModeOptionStruct("Light", Clusters::DishwasherMode::ModeLight, List<const ModeTagStructType>(modeTagsLight)),
    };

    CHIP_ERROR AppInit() override;
    void HandleChangeToMode(uint8_t mode, ModeBase::Commands::ChangeToModeResponse::Type & response) override;

    uint8_t NumberOfModes() override { return 3; };
    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> & tags) override;

public:
    DishwasherModeInstance(EndpointId aEndpointId, ClusterId aClusterId, uint32_t aFeature) :
        Instance(aEndpointId, aClusterId, aFeature){};

    ~DishwasherModeInstance() override = default;
};

} // namespace DishwasherMode

} // namespace Clusters
} // namespace app
} // namespace chip
