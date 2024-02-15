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

namespace chip {
namespace app {
namespace Clusters {

class RvcDevice;

typedef void (RvcDevice::*HandleChangeToModeA)(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response);

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

    RvcDevice * mRvcDeviceInstance;
    HandleChangeToModeA mCallback;

public:
    void SetHandleChangeToMode(HandleChangeToModeA aCallback, RvcDevice * aInstance)
    {
        mCallback          = aCallback;
        mRvcDeviceInstance = aInstance;
    };

    ~RvcRunModeDelegate() override = default;
};

void Shutdown();

} // namespace RvcRunMode

namespace RvcCleanMode {

const uint8_t ModeQuick     = 0;
const uint8_t ModeAuto      = 1;
const uint8_t ModeDeepClean = 2;
const uint8_t ModeQuiet     = 3;
const uint8_t ModeMaxVac    = 4;

/// This is an application level delegate to handle RvcClean commands according to the specific business logic.
class RvcCleanModeDelegate : public ModeBase::Delegate
{
private:
    using ModeTagStructType            = detail::Structs::ModeTagStruct::Type;
    ModeTagStructType modeTagsQuick[2] = { { .value = to_underlying(ModeTag::kVacuum) },
                                           { .value = to_underlying(ModeBase::ModeTag::kQuick) } };

    ModeTagStructType modeTagsAuto[2] = { { .value = to_underlying(ModeBase::ModeTag::kAuto) },
                                          { .value = to_underlying(ModeTag::kVacuum) } };

    ModeTagStructType modeTagsDeepClean[3] = { { .value = to_underlying(ModeTag::kMop) },
                                               { .value = to_underlying(ModeTag::kDeepClean) },
                                               { .value = to_underlying(ModeTag::kVacuum) } };

    ModeTagStructType modeTagsQuiet[2] = { { .value = to_underlying(ModeBase::ModeTag::kQuiet) },
                                           { .value = to_underlying(ModeTag::kVacuum) } };

    ModeTagStructType modeTagsMaxVac[2] = { { .value = to_underlying(ModeTag::kVacuum) },
                                            { .value = to_underlying(ModeTag::kDeepClean) } };

    const detail::Structs::ModeOptionStruct::Type kModeOptions[5] = {
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Quick"),
                                                 .mode     = ModeQuick,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(modeTagsQuick) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Auto"),
                                                 .mode     = ModeAuto,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(modeTagsAuto) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Deep Clean"),
                                                 .mode     = ModeDeepClean,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(modeTagsDeepClean) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Quiet"),
                                                 .mode     = ModeQuiet,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(modeTagsQuiet) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Max Vac"),
                                                 .mode     = ModeMaxVac,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(modeTagsMaxVac) },
    };

    CHIP_ERROR Init() override;
    void HandleChangeToMode(uint8_t mode, ModeBase::Commands::ChangeToModeResponse::Type & response) override;

    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<ModeTagStructType> & tags) override;

    RvcDevice * mRvcDeviceInstance;
    HandleChangeToModeA mCallback;

public:
    void SetHandleChangeToMode(HandleChangeToModeA aCallback, RvcDevice * aInstance)
    {
        mCallback          = aCallback;
        mRvcDeviceInstance = aInstance;
    };

    ~RvcCleanModeDelegate() override = default;
};

void Shutdown();

} // namespace RvcCleanMode

} // namespace Clusters
} // namespace app
} // namespace chip
