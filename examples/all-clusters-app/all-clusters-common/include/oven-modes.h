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

namespace OvenMode {

const uint8_t ModeBake            = 0;
const uint8_t ModeConvection      = 1;
const uint8_t ModeGrill           = 2;
const uint8_t ModeRoast           = 3;
const uint8_t ModeClean           = 4;
const uint8_t ModeConvectionBake  = 5;
const uint8_t ModeConvectionRoast = 6;
const uint8_t ModeWarming         = 7;
const uint8_t ModeProofing        = 8;

/// This is an application level delegate to handle Oven commands according to the specific business logic.
class OvenModeDelegate : public ModeBase::Delegate
{
private:
    using ModeTagStructType                      = detail::Structs::ModeTagStruct::Type;
    ModeTagStructType ModeTagsBake[1]            = { { .value = to_underlying(ModeTag::kBake) } };
    ModeTagStructType ModeTagsConvection[1]      = { { .value = to_underlying(ModeTag::kConvection) } };
    ModeTagStructType ModeTagsGrill[1]           = { { .value = to_underlying(ModeTag::kGrill) } };
    ModeTagStructType ModeTagsRoast[1]           = { { .value = to_underlying(ModeTag::kRoast) } };
    ModeTagStructType ModeTagsClean[1]           = { { .value = to_underlying(ModeTag::kClean) } };
    ModeTagStructType ModeTagsConvectionBake[1]  = { { .value = to_underlying(ModeTag::kConvectionBake) } };
    ModeTagStructType ModeTagsConvectionRoast[1] = { { .value = to_underlying(ModeTag::kConvectionRoast) } };
    ModeTagStructType ModeTagsWarming[1]         = { { .value = to_underlying(ModeTag::kWarming) } };
    ModeTagStructType ModeTagsProofing[1]        = { { .value = to_underlying(ModeTag::kProofing) } };

    const detail::Structs::ModeOptionStruct::Type kModeOptions[9] = {
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Bake"),
                                                 .mode     = ModeBake,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsBake) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Convection"),
                                                 .mode     = ModeConvection,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsConvection) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Grill"),
                                                 .mode     = ModeGrill,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsGrill) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Roast"),
                                                 .mode     = ModeRoast,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsRoast) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Clean"),
                                                 .mode     = ModeClean,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsClean) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Convection Bake"),
                                                 .mode     = ModeConvectionBake,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsConvectionBake) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Convection Roast"),
                                                 .mode     = ModeConvectionRoast,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsConvectionRoast) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Warming"),
                                                 .mode     = ModeWarming,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsWarming) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Proofing"),
                                                 .mode     = ModeProofing,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsProofing) },
    };

    CHIP_ERROR Init() override;
    void HandleChangeToMode(uint8_t mode, ModeBase::Commands::ChangeToModeResponse::Type & response) override;

    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<ModeTagStructType> & tags) override;

public:
    ~OvenModeDelegate() override = default;
};

ModeBase::Instance * Instance();

void Shutdown();

} // namespace OvenMode
} // namespace Clusters
} // namespace app
} // namespace chip
