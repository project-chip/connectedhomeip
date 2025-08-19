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

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {

namespace RvcRunMode {

const uint8_t ModeIdle     = 0;
const uint8_t ModeCleaning = 1;
const uint8_t ModeMapping  = 2;

/// This is an application level delegate to handle RvcRun commands according to the specific business logic.
class RvcRunModeDelegate : public ModeBase::Delegate
{
private:
    using ModeTagStructType               = detail::Structs::ModeTagStruct::Type;
    ModeTagStructType ModeTagsIdle[1]     = { { .value = to_underlying(ModeTag::kIdle) } };
    ModeTagStructType ModeTagsCleaning[1] = { { .value = to_underlying(ModeTag::kCleaning) } };
    ModeTagStructType ModeTagsMapping[1]  = { { .value = to_underlying(ModeTag::kMapping) } };

    const detail::Structs::ModeOptionStruct::Type kModeOptions[3] = {
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Idle"),
                                                 .mode     = ModeIdle,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsIdle) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Cleaning"),
                                                 .mode     = ModeCleaning,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsCleaning) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Mapping"),
                                                 .mode     = ModeMapping,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsMapping) },
    };

    CHIP_ERROR Init() override;
    void HandleChangeToMode(uint8_t mode, ModeBase::Commands::ChangeToModeResponse::Type & response) override;

    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<ModeTagStructType> & tags) override;

public:
    ~RvcRunModeDelegate() override = default;
};

ModeBase::Instance * Instance();

void Shutdown();

} // namespace RvcRunMode

namespace RvcCleanMode {

const uint8_t ModeVacuum    = 0;
const uint8_t ModeWash      = 1;
const uint8_t ModeDeepClean = 2;

/// This is an application level delegate to handle RvcClean commands according to the specific business logic.
class RvcCleanModeDelegate : public ModeBase::Delegate
{
private:
    using ModeTagStructType            = detail::Structs::ModeTagStruct::Type;
    ModeTagStructType modeTagsVac[1]   = { { .value = to_underlying(ModeTag::kVacuum) } };
    ModeTagStructType modeTagsMop[1]   = { { .value = to_underlying(ModeTag::kMop) } };
    ModeTagStructType modeTagsBoost[2] = { { .value = to_underlying(ModeBase::ModeTag::kMax) },
                                           { .value = to_underlying(ModeTag::kDeepClean) } };

    const detail::Structs::ModeOptionStruct::Type kModeOptions[3] = {
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Vacuum"),
                                                 .mode     = ModeVacuum,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(modeTagsVac) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Wash"),
                                                 .mode     = ModeWash,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(modeTagsMop) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Deep clean"),
                                                 .mode     = ModeDeepClean,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(modeTagsBoost) },
    };

    CHIP_ERROR Init() override;
    void HandleChangeToMode(uint8_t mode, ModeBase::Commands::ChangeToModeResponse::Type & response) override;

    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<ModeTagStructType> & tags) override;

public:
    ~RvcCleanModeDelegate() override = default;
};

ModeBase::Instance * Instance();

void Shutdown();

} // namespace RvcCleanMode

} // namespace Clusters
} // namespace app
} // namespace chip

#ifdef MATTER_DM_PLUGIN_RVC_RUN_MODE_SERVER
chip::app::Clusters::ModeBase::Instance * getRvcRunModeInstance();

chip::Protocols::InteractionModel::Status chefRvcRunModeWriteCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                                      const EmberAfAttributeMetadata * attributeMetadata,
                                                                      uint8_t * buffer);
chip::Protocols::InteractionModel::Status chefRvcRunModeReadCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                                     const EmberAfAttributeMetadata * attributeMetadata,
                                                                     uint8_t * buffer, uint16_t maxReadLength);
#endif // MATTER_DM_PLUGIN_RVC_RUN_MODE_SERVER

#ifdef MATTER_DM_PLUGIN_RVC_CLEAN_MODE_SERVER
chip::Protocols::InteractionModel::Status chefRvcCleanModeWriteCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                                        const EmberAfAttributeMetadata * attributeMetadata,
                                                                        uint8_t * buffer);
chip::Protocols::InteractionModel::Status chefRvcCleanModeReadCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                                       const EmberAfAttributeMetadata * attributeMetadata,
                                                                       uint8_t * buffer, uint16_t maxReadLength);
#endif // MATTER_DM_PLUGIN_RVC_CLEAN_MODE_SERVER
