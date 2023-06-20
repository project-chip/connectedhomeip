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

#include <app/clusters/mode-base-server/mode-base-delegate.h>
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

namespace RvcRunMode {

const uint8_t ModeIdle = 0;
const uint8_t ModeCleaning = 1;
const uint8_t ModeMapping = 2;

/// This is an application level delegate to handle RvcRun commands according to the specific business logic.
class RvcRunModeDelegate : public ModeBase::Delegate
{
private:
    ModeTagStructType ModeTagsIdle[1]     = { { .value = static_cast<uint16_t>(Clusters::RvcRunMode::ModeTag::kIdle) } };
    ModeTagStructType ModeTagsCleaning[1] = { { .value = static_cast<uint16_t>(Clusters::RvcRunMode::ModeTag::kCleaning) } };

    const ModeOptionStructType modeOptions[3] = {
        Delegate::BuildModeOptionStruct("Idle", Clusters::RvcRunMode::ModeIdle, List<const ModeTagStructType>(ModeTagsIdle)),
        Delegate::BuildModeOptionStruct("Cleaning", Clusters::RvcRunMode::ModeCleaning, List<const ModeTagStructType>(ModeTagsCleaning)),
        Delegate::BuildModeOptionStruct("Mapping", Clusters::RvcRunMode::ModeMapping, List<const ModeTagStructType>(ModeTagsIdle)), // todo set to no mode tags
    };

    CHIP_ERROR Init() override;
    void HandleChangeToMode(uint8_t mode, ModeBase::Commands::ChangeToModeResponse::Type &response) override;

    uint8_t NumberOfModes() override {return 3;};
    CHIP_ERROR getModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan &label) override;
    CHIP_ERROR getModeValueByIndex(uint8_t modeIndex, uint8_t &value) override;
    CHIP_ERROR getModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> &tags) override;

public:
    RvcRunModeDelegate() = default;

    ~RvcRunModeDelegate() override = default;
};

} // namespace RvcRunMode

namespace RvcCleanMode {

const uint8_t ModeVacuum = 0;
const uint8_t ModeWash = 1;
const uint8_t ModeDeepClean = 2;

/// This is an application level delegate to handle RvcClean commands according to the specific business logic.
class RvcCleanModeDelegate : public ModeBase::Delegate
{
private:
    ModeTagStructType modeTagsVac[1]   = { { .value = static_cast<uint16_t>(Clusters::RvcCleanMode::ModeTag::kVacuum) } };
    ModeTagStructType modeTagsMop[1]   = { { .value = static_cast<uint16_t>(Clusters::RvcCleanMode::ModeTag::kMop) } };
    ModeTagStructType modeTagsBoost[2] = { { .value = static_cast<uint16_t>(Clusters::ModeBase::ModeTag::kMax) },
                                          { .value = static_cast<uint16_t>(Clusters::RvcCleanMode::ModeTag::kDeepClean) }};

    const ModeOptionStructType modeOptions[3] = {
        Delegate::BuildModeOptionStruct("Vacuum", Clusters::RvcCleanMode::ModeVacuum, List<const ModeTagStructType>(modeTagsVac)),
        Delegate::BuildModeOptionStruct("Wash", Clusters::RvcCleanMode::ModeWash, List<const ModeTagStructType>(modeTagsMop)),
        Delegate::BuildModeOptionStruct("Deep clean", Clusters::RvcCleanMode::ModeDeepClean, List<const ModeTagStructType>(modeTagsBoost)),
    };

    CHIP_ERROR Init() override;
    void HandleChangeToMode(uint8_t mode, ModeBase::Commands::ChangeToModeResponse::Type &response) override;

    uint8_t NumberOfModes() override {return 3;};
    CHIP_ERROR getModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan &label) override;
    CHIP_ERROR getModeValueByIndex(uint8_t modeIndex, uint8_t &value) override;
    CHIP_ERROR getModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> &tags) override;

public:
    RvcCleanModeDelegate() = default;

    ~RvcCleanModeDelegate() override = default;
};

} // namespace RvcCleanMode

} // namespace Clusters
} // namespace app
} // namespace chip
