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

namespace RefrigeratorAndTemperatureControlledCabinetMode {

const uint8_t ModeNormal      = 0;
const uint8_t ModeRapidCool   = 1;
const uint8_t ModeRapidFreeze = 2;

/// This is an application level delegate to handle RefrigeratorAndTemperatureControlledCabinet commands according to the specific
/// business logic.
class TccModeDelegate : public ModeBase::Delegate
{
private:
    using ModeTagStructType                     = detail::Structs::ModeTagStruct::Type;
    ModeTagStructType modeTagsTccNormal[1]      = { { .value = to_underlying(ModeBase::ModeTag::kAuto) } };
    ModeTagStructType modeTagsTccRapidCool[1]   = { { .value = to_underlying(ModeTag::kRapidCool) } };
    ModeTagStructType modeTagsTccRapidFreeze[1] = { { .value = to_underlying(ModeTag::kRapidFreeze) } };

    const detail::Structs::ModeOptionStruct::Type kModeOptions[3] = {
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Normal"),
                                                 .mode     = ModeNormal,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(modeTagsTccNormal) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Rapid Cool"),
                                                 .mode     = ModeRapidCool,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(modeTagsTccRapidCool) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Rapid Freeze"),
                                                 .mode     = ModeRapidFreeze,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(modeTagsTccRapidFreeze) },
    };

    CHIP_ERROR Init() override;
    void HandleChangeToMode(uint8_t mode, ModeBase::Commands::ChangeToModeResponse::Type & response) override;

    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<ModeTagStructType> & tags) override;

public:
    ~TccModeDelegate() override = default;
};

void Shutdown();

} // namespace RefrigeratorAndTemperatureControlledCabinetMode

} // namespace Clusters
} // namespace app
} // namespace chip
