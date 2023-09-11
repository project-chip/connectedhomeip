/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

namespace LaundryWasherMode {

const uint8_t ModeNormal   = 0;
const uint8_t ModeDelicate = 1;
const uint8_t ModeHeavy    = 2;
const uint8_t ModeWhites   = 3;

/// This is an application level delegate to handle LaundryWasherMode commands according to the specific business logic.
class LaundryWasherModeDelegate : public ModeBase::Delegate
{
private:
    using ModeTagStructType               = detail::Structs::ModeTagStruct::Type;
    ModeTagStructType modeTagsNormal[1]   = { { .value = to_underlying(ModeTag::kNormal) } };
    ModeTagStructType modeTagsDelicate[3] = { { .value = to_underlying(ModeTag::kDelicate) },
                                              { .value = to_underlying(ModeBase::ModeTag::kNight) },
                                              { .value = to_underlying(ModeBase::ModeTag::kQuiet) } };
    ModeTagStructType modeTagsHeavy[2]    = { { .value = to_underlying(ModeBase::ModeTag::kMax) },
                                           { .value = to_underlying(ModeTag::kHeavy) } };
    ModeTagStructType modeTagsWhites[1]   = { { .value = to_underlying(ModeTag::kWhites) } };

    const detail::Structs::ModeOptionStruct::Type kModeOptions[4] = {
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Normal"),
                                                 .mode     = ModeNormal,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(modeTagsNormal) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Delicate"),
                                                 .mode     = ModeDelicate,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(modeTagsDelicate) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Heavy"),
                                                 .mode     = ModeHeavy,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(modeTagsHeavy) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Whites"),
                                                 .mode     = ModeWhites,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(modeTagsWhites) },
    };

    CHIP_ERROR Init() override;
    void HandleChangeToMode(uint8_t mode, ModeBase::Commands::ChangeToModeResponse::Type & response) override;

    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<ModeTagStructType> & tags) override;

public:
    ~LaundryWasherModeDelegate() override = default;
};

ModeBase::Instance * Instance();

void Shutdown();

} // namespace LaundryWasherMode

} // namespace Clusters
} // namespace app
} // namespace chip
