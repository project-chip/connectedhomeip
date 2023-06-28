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

namespace RvcRunMode {

const uint8_t ModeIdle     = 0;
const uint8_t ModeCleaning = 1;
const uint8_t ModeMapping  = 2;

/// This is an application level delegate to handle RvcRun commands according to the specific business logic.
class RvcRunModeInstance : public ModeBase::Instance
{
private:
    ModeTagStructType ModeTagsIdle[1]     = { { .value = static_cast<uint16_t>(Clusters::RvcRunMode::ModeTag::kIdle) } };
    ModeTagStructType ModeTagsCleaning[1] = { { .value = static_cast<uint16_t>(Clusters::RvcRunMode::ModeTag::kCleaning) } };

    const ModeOptionStructType modeOptions[3] = {
        BuildModeOptionStruct("Idle", Clusters::RvcRunMode::ModeIdle, List<const ModeTagStructType>(ModeTagsIdle)),
        BuildModeOptionStruct("Cleaning", Clusters::RvcRunMode::ModeCleaning, List<const ModeTagStructType>(ModeTagsCleaning)),
        BuildModeOptionStruct("Mapping", Clusters::RvcRunMode::ModeMapping,
                              List<const ModeTagStructType>(ModeTagsIdle)), // todo set to no mode tags
    };

    CHIP_ERROR AppInit() override;
    void HandleChangeToMode(uint8_t mode, ModeBase::Commands::ChangeToModeResponse::Type & response) override;

    uint8_t NumberOfModes() override { return ArraySize(modeOptions); };
    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> & tags) override;

public:
    RvcRunModeInstance(EndpointId aEndpointId, ClusterId aClusterId, uint32_t aFeature) :
        Instance(aEndpointId, aClusterId, aFeature){};

    ~RvcRunModeInstance() override = default;
};

} // namespace RvcRunMode

namespace RvcCleanMode {

const uint8_t ModeVacuum    = 0;
const uint8_t ModeWash      = 1;
const uint8_t ModeDeepClean = 2;

/// This is an application level delegate to handle RvcClean commands according to the specific business logic.
class RvcCleanModeInstance : public ModeBase::Instance
{
private:
    ModeTagStructType modeTagsVac[1]   = { { .value = static_cast<uint16_t>(Clusters::RvcCleanMode::ModeTag::kVacuum) } };
    ModeTagStructType modeTagsMop[1]   = { { .value = static_cast<uint16_t>(Clusters::RvcCleanMode::ModeTag::kMop) } };
    ModeTagStructType modeTagsBoost[2] = { { .value = static_cast<uint16_t>(Clusters::ModeBase::ModeTag::kMax) },
                                           { .value = static_cast<uint16_t>(Clusters::RvcCleanMode::ModeTag::kDeepClean) } };

    const ModeOptionStructType modeOptions[3] = {
        BuildModeOptionStruct("Vacuum", Clusters::RvcCleanMode::ModeVacuum, List<const ModeTagStructType>(modeTagsVac)),
        BuildModeOptionStruct("Wash", Clusters::RvcCleanMode::ModeWash, List<const ModeTagStructType>(modeTagsMop)),
        BuildModeOptionStruct("Deep clean", Clusters::RvcCleanMode::ModeDeepClean, List<const ModeTagStructType>(modeTagsBoost)),
    };

    CHIP_ERROR AppInit() override;
    void HandleChangeToMode(uint8_t mode, ModeBase::Commands::ChangeToModeResponse::Type & response) override;

    uint8_t NumberOfModes() override { return ArraySize(modeOptions); };
    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> & tags) override;

public:
    RvcCleanModeInstance(EndpointId aEndpointId, ClusterId aClusterId, uint32_t aFeature) :
        Instance(aEndpointId, aClusterId, aFeature){};

    ~RvcCleanModeInstance() override = default;
};

} // namespace RvcCleanMode

} // namespace Clusters
} // namespace app
} // namespace chip
