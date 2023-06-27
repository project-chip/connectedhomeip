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
using List               = chip::app::DataModel::List<T>;
using ModeTagStructType = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;
using ModeOptionStructType = chip::app::Clusters::detail::Structs::ModeOptionStruct::Type;

namespace chip {
namespace app {
namespace Clusters {

namespace RefrigeratorAndTemperatureControlledCabinetMode {

const uint8_t ModeNormal = 0;
const uint8_t ModeRapidCool = 1;
const uint8_t ModeRapidFreeze = 2;

/// This is an application level delegate to handle RefrigeratorAndTemperatureControlledCabinet commands according to the specific business logic.
class TccModeInstance : public ModeBase::Instance
{
private:
    ModeTagStructType modeTagsTccNormal[1] = { { .value = static_cast<uint16_t>(Clusters::ModeBase::ModeTag::kAuto) } };
    ModeTagStructType modeTagsTccRapidCool[1] = { { .value = static_cast<uint16_t>(Clusters::RefrigeratorAndTemperatureControlledCabinetMode::ModeTag::kRapidCool) }};
    ModeTagStructType modeTagsTccRapidFreeze[1] = { { .value = static_cast<uint16_t>(Clusters::RefrigeratorAndTemperatureControlledCabinetMode::ModeTag::kRapidFreeze) }};

    const ModeOptionStructType modeOptions[3] = {
        BuildModeOptionStruct("Normal", Clusters::RefrigeratorAndTemperatureControlledCabinetMode::ModeNormal, List<const ModeTagStructType>(modeTagsTccNormal)),
        BuildModeOptionStruct("Rapid Cool", Clusters::RefrigeratorAndTemperatureControlledCabinetMode::ModeRapidCool, List<const ModeTagStructType>(modeTagsTccRapidCool)),
        BuildModeOptionStruct("Rapid Freeze", Clusters::RefrigeratorAndTemperatureControlledCabinetMode::ModeRapidFreeze, List<const ModeTagStructType>(modeTagsTccRapidFreeze)),
    };

    CHIP_ERROR AppInit() override;
    void HandleChangeToMode(uint8_t mode, ModeBase::Commands::ChangeToModeResponse::Type &response) override;

    uint8_t NumberOfModes() override {return 3;};
    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan &label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t &value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> &tags) override;

public:
    TccModeInstance(EndpointId aEndpointId, ClusterId aClusterId, uint32_t aFeature) : Instance(aEndpointId, aClusterId, aFeature) {};

    ~TccModeInstance() override = default;
};

} // namespace RefrigeratorAndTemperatureControlledCabinetMode

} // namespace Clusters
} // namespace app
} // namespace chip
