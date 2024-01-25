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
#include <app/util/af.h>
#include <app/util/config.h>
#include <cstring>
#include <utility>

namespace chip {
namespace app {
namespace Clusters {

namespace EnergyEvseMode {

constexpr uint8_t kModeManual                    = 0;
constexpr uint8_t kModeTimeOfUse                 = 1;
constexpr uint8_t kModeSolarCharging             = 2;
constexpr uint8_t kModeTimeOfUseAndSolarCharging = 3;

/// This is an application level delegate to handle EnergyEvse modes.
class EnergyEvseModeDelegate : public ModeBase::Delegate
{
private:
    using ModeTagStructType                                = detail::Structs::ModeTagStruct::Type;
    ModeTagStructType ModeTagsManual[1]                    = { { .value = to_underlying(ModeTag::kManual) } };
    ModeTagStructType ModeTagsTimeOfUse[1]                 = { { .value = to_underlying(ModeTag::kTimeOfUse) } };
    ModeTagStructType ModeTagsSolarCharging[1]             = { { .value = to_underlying(ModeTag::kSolarCharging) } };
    ModeTagStructType ModeTagsTimeOfUseAndSolarCharging[2] = {
        { .value = to_underlying(ModeTag::kTimeOfUse) },
        { .value = to_underlying(ModeTag::kSolarCharging) },
    };

    const detail::Structs::ModeOptionStruct::Type kModeOptions[4] = {
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Manual"),
                                                 .mode     = kModeManual,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsManual) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Auto-scheduled"),
                                                 .mode     = kModeTimeOfUse,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsTimeOfUse) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Solar"),
                                                 .mode     = kModeSolarCharging,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsSolarCharging) },
        detail::Structs::ModeOptionStruct::Type{ .label = CharSpan::fromCharString("Auto-scheduled with Solar charging"),
                                                 .mode  = kModeTimeOfUseAndSolarCharging,
                                                 .modeTags =
                                                     DataModel::List<const ModeTagStructType>(ModeTagsTimeOfUseAndSolarCharging) },

    };

    CHIP_ERROR Init() override;
    void HandleChangeToMode(uint8_t mode, ModeBase::Commands::ChangeToModeResponse::Type & response) override;

    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<ModeTagStructType> & tags) override;

public:
    ~EnergyEvseModeDelegate() override = default;
};

ModeBase::Instance * Instance();

void Shutdown();

} // namespace EnergyEvseMode

} // namespace Clusters
} // namespace app
} // namespace chip
