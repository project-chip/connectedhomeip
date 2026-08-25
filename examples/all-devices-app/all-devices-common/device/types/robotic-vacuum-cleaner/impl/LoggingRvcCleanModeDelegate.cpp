/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <device/types/robotic-vacuum-cleaner/impl/LoggingRvcCleanModeDelegate.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <algorithm>

namespace chip::app::Clusters::RvcCleanMode {

namespace {

using ModeTagStructType = detail::Structs::ModeTagStruct::Type;

constexpr uint8_t kRunModeIdle = 0;

const ModeTagStructType kQuickTags[] = { { .value = to_underlying(ModeTag::kVacuum) },
                                         { .value = to_underlying(ModeTag::kQuick) } };
const ModeTagStructType kAutoTags[]  = { { .value = to_underlying(ModeTag::kAuto) }, { .value = to_underlying(ModeTag::kVacuum) } };
const ModeTagStructType kDeepCleanTags[]  = { { .value = to_underlying(ModeTag::kMop) },
                                              { .value = to_underlying(ModeTag::kDeepClean) },
                                              { .value = to_underlying(ModeTag::kVacuum) } };
const ModeTagStructType kQuietTags[]      = { { .value = to_underlying(ModeTag::kQuiet) },
                                              { .value = to_underlying(ModeTag::kVacuum) } };
const ModeTagStructType kMaxVacTags[]     = { { .value = to_underlying(ModeTag::kVacuum) },
                                              { .value = to_underlying(ModeTag::kDeepClean) } };
const ModeTagStructType kVacThenMopTags[] = { { .value = to_underlying(ModeTag::kVacuum) },
                                              { .value = to_underlying(ModeTag::kMop) },
                                              { .value = to_underlying(ModeTag::kVacuumThenMop) } };

struct ModeOption
{
    CharSpan label;
    uint8_t value;
    Span<const ModeTagStructType> tags;
};

const ModeOption kModeOptions[] = {
    { "Quick"_span, 0, Span<const ModeTagStructType>(kQuickTags) },
    { "Auto"_span, 1, Span<const ModeTagStructType>(kAutoTags) },
    { "Deep Clean"_span, 2, Span<const ModeTagStructType>(kDeepCleanTags) },
    { "Quiet"_span, 3, Span<const ModeTagStructType>(kQuietTags) },
    { "Max Vac"_span, 4, Span<const ModeTagStructType>(kMaxVacTags) },
    { "Vacuum Then Mop"_span, 5, Span<const ModeTagStructType>(kVacThenMopTags) },
};

} // namespace

CHIP_ERROR LoggingRvcCleanModeDelegate::GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kModeOptions), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    return CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
}

CHIP_ERROR LoggingRvcCleanModeDelegate::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kModeOptions), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    value = kModeOptions[modeIndex].value;
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingRvcCleanModeDelegate::GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<ModeTagStructType> & modeTags)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kModeOptions), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    const auto & tags = kModeOptions[modeIndex].tags;
    VerifyOrReturnError(modeTags.size() >= tags.size(), CHIP_ERROR_INVALID_ARGUMENT);
    std::copy(tags.begin(), tags.end(), modeTags.begin());
    modeTags.reduce_size(tags.size());
    return CHIP_NO_ERROR;
}

void LoggingRvcCleanModeDelegate::HandleChangeToMode(uint8_t newMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    ChipLogProgress(Zcl, "LoggingRvcCleanModeDelegate: ChangeToMode(%u) received.", newMode);

    if (mRunModeCluster.GetCurrentMode() != kRunModeIdle)
    {
        response.status = to_underlying(ModeBase::StatusCode::kInvalidInMode);
        response.statusText.SetValue("Change of the cleaning mode is only allowed in Idle."_span);
        return;
    }

    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
}

} // namespace chip::app::Clusters::RvcCleanMode
