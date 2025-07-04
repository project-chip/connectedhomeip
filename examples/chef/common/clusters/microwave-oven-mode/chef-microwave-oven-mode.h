/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/util/config.h>
#include <cstring>
#include <utility>

#pragma once

#ifdef MATTER_DM_PLUGIN_MICROWAVE_OVEN_MODE_SERVER

namespace chip {
namespace app {
namespace Clusters {
namespace MicrowaveOvenMode {

const uint8_t ModeNormal  = 0;
const uint8_t ModeDefrost = 1;

class ChefDelegate : public ModeBase::Delegate
{
private:
    using ModeTagStructType              = detail::Structs::ModeTagStruct::Type;
    ModeTagStructType ModeTagsNormal[1]  = { { .value = to_underlying(ModeTag::kNormal) } };
    ModeTagStructType ModeTagsDefrost[1] = { { .value = to_underlying(ModeTag::kDefrost) } };

    const detail::Structs::ModeOptionStruct::Type kModeOptions[2] = {
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Normal"),
                                                 .mode     = ModeNormal,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsNormal) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Defrost"),
                                                 .mode     = ModeDefrost,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsDefrost) },
    };

public:
    CHIP_ERROR Init() override;

    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;

    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;

    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<detail::Structs::ModeTagStruct::Type> & modeTags) override;

    void HandleChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response) override;

    ~ChefDelegate() = default;
    ChefDelegate()  = default;
};

} // namespace MicrowaveOvenMode
} // namespace Clusters
} // namespace app
} // namespace chip

namespace ChefMicrowaveOvenMode {
void InitChefMicrowaveOvenModeCluster();
chip::app::Clusters::ModeBase::Instance * GetInstance(chip::EndpointId endpointId);
} // namespace ChefMicrowaveOvenMode

#endif // MATTER_DM_PLUGIN_MICROWAVE_OVEN_MODE_SERVER
