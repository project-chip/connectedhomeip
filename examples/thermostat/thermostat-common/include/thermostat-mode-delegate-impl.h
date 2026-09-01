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

#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/util/af-types.h>
#include <clusters/ThermostatMode/Enums.h>
#include <clusters/ThermostatMode/Ids.h>
#include <cstring>
#include <utility>

namespace chip {
namespace app {
namespace Clusters {

namespace ThermostatMode {

inline constexpr uint8_t ModeOff           = 0;
inline constexpr uint8_t ModeCool          = 1;
inline constexpr uint8_t ModeHeat          = 2;
inline constexpr uint8_t ModeEmergencyHeat = 3;
inline constexpr uint8_t ModeAuto          = 4;

/// Application level delegate to handle ThermostatMode commands according to business logic.
class ThermostatModeDelegate : public ModeBase::Delegate
{
private:
    using ModeTagStructType = detail::Structs::ModeTagStruct::Type;

    ModeTagStructType ModeTagsOff[1]           = { { .value = to_underlying(ModeTag::kOff) } };
    ModeTagStructType ModeTagsCool[1]          = { { .value = to_underlying(ModeTag::kCool) } };
    ModeTagStructType ModeTagsHeat[1]          = { { .value = to_underlying(ModeTag::kHeat) } };
    ModeTagStructType ModeTagsEmergencyHeat[2] = { { .value = to_underlying(ModeTag::kHeat) },
                                                  { .value = to_underlying(ModeTag::kEmergencyHeat) } };
    ModeTagStructType ModeTagsAuto[1]          = { { .value = to_underlying(ModeTag::kAuto) } };

    const detail::Structs::ModeOptionStruct::Type kModeOptions[5] = {
        detail::Structs::ModeOptionStruct::Type{
            .label    = "Off"_span,
            .mode     = ModeOff,
            .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsOff),
        },
        detail::Structs::ModeOptionStruct::Type{
            .label    = "Cool"_span,
            .mode     = ModeCool,
            .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsCool),
        },
        detail::Structs::ModeOptionStruct::Type{
            .label    = "Heat"_span,
            .mode     = ModeHeat,
            .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsHeat),
        },
        detail::Structs::ModeOptionStruct::Type{
            .label    = "Emergency Heat"_span,
            .mode     = ModeEmergencyHeat,
            .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsEmergencyHeat),
        },
        detail::Structs::ModeOptionStruct::Type{
            .label    = "Auto"_span,
            .mode     = ModeAuto,
            .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsAuto),
        },
    };

    CHIP_ERROR Init() override;
    void HandleChangeToMode(uint8_t mode, ModeBase::Commands::ChangeToModeResponse::Type & response) override;
    void HandleChangeToModeByCoreTag(uint16_t newModeTag, uint8_t & newMode,
                                    ModeBase::Commands::ChangeToModeResponse::Type & response) override;

    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<ModeTagStructType> & tags) override;
    CHIP_ERROR GetCoreModeTagByIndex(uint8_t tagIndex, uint16_t & tag) override;

public:
    ~ThermostatModeDelegate() override = default;
};

ModeBase::Instance * Instance();

void Shutdown();

CHIP_ERROR Init(EndpointId endpointId = 1);

} // namespace ThermostatMode

namespace Thermostat {
using ThermostatModeDelegate = ThermostatMode::ThermostatModeDelegate;
} // namespace Thermostat

} // namespace Clusters
} // namespace app
} // namespace chip

void MatterThermostatModeClusterInitCallback(chip::EndpointId endpointId);
void MatterThermostatModeClusterShutdownCallback(chip::EndpointId endpointId, MatterClusterShutdownType shutdownType);
