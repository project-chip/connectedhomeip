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
#include <app/util/config.h>
#include <cstring>
#include <utility>

namespace chip {
namespace app {
namespace Clusters {

namespace WaterHeaterMode {

constexpr uint8_t kModeOff    = 0;
constexpr uint8_t kModeManual = 1;
constexpr uint8_t kModeTimed  = 2;

/// This is an application level delegate to handle WaterHeaterMode commands according to the specific business logic.
class ExampleWaterHeaterModeDelegate : public ModeBase::Delegate
{
private:
    using ModeTagStructType             = detail::Structs::ModeTagStruct::Type;
    ModeTagStructType modeTagsOff[1]    = { { .value = to_underlying(ModeTag::kOff) } };
    ModeTagStructType modeTagsManual[1] = { { .value = to_underlying(ModeTag::kManual) } };
    ModeTagStructType modeTagsTimed[1]  = { { .value = to_underlying(ModeTag::kTimed) } };

    const detail::Structs::ModeOptionStruct::Type kModeOptions[3] = {
        detail::Structs::ModeOptionStruct::Type{
            .label = "Off"_span, .mode = kModeOff, .modeTags = DataModel::List<const ModeTagStructType>(modeTagsOff) },
        detail::Structs::ModeOptionStruct::Type{
            .label = "Manual"_span, .mode = kModeManual, .modeTags = DataModel::List<const ModeTagStructType>(modeTagsManual) },
        detail::Structs::ModeOptionStruct::Type{
            .label = "Timed"_span, .mode = kModeTimed, .modeTags = DataModel::List<const ModeTagStructType>(modeTagsTimed) }
    };

    CHIP_ERROR Init() override;
    void HandleChangeToMode(uint8_t mode, ModeBase::Commands::ChangeToModeResponse::Type & response) override;

    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<ModeTagStructType> & tags) override;

public:
    ~ExampleWaterHeaterModeDelegate() override = default;
};

ModeBase::Instance * Instance();

void Shutdown();

} // namespace WaterHeaterMode

} // namespace Clusters
} // namespace app
} // namespace chip
