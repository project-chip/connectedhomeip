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

#include "LoggingRvcRunModeDelegate.h"
#include <device/capabilities/service-area/impl/LoggingServiceAreaDelegate.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <algorithm>

namespace chip::app::Clusters::RvcRunMode {

namespace {

using ModeTagStructType = detail::Structs::ModeTagStruct::Type;

constexpr uint8_t kModeIdle     = 0;
constexpr uint8_t kModeCleaning = 1;
constexpr uint8_t kModeMapping  = 2;

const ModeTagStructType kIdleTags[]     = { { .value = to_underlying(ModeTag::kIdle) } };
const ModeTagStructType kCleaningTags[] = { { .value = to_underlying(ModeTag::kCleaning) } };
const ModeTagStructType kMappingTags[]  = { { .value = to_underlying(ModeTag::kMapping) } };

struct ModeOption
{
    CharSpan label;
    uint8_t value;
    Span<const ModeTagStructType> tags;
};

const ModeOption kModeOptions[] = {
    { "Idle"_span, kModeIdle, Span<const ModeTagStructType>(kIdleTags) },
    { "Cleaning"_span, kModeCleaning, Span<const ModeTagStructType>(kCleaningTags) },
    { "Mapping"_span, kModeMapping, Span<const ModeTagStructType>(kMappingTags) },
};

} // namespace

CHIP_ERROR LoggingRvcRunModeDelegate::GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kModeOptions), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    return CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
}

CHIP_ERROR LoggingRvcRunModeDelegate::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kModeOptions), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    value = kModeOptions[modeIndex].value;
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingRvcRunModeDelegate::GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<ModeTagStructType> & modeTags)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kModeOptions), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    const auto & tags = kModeOptions[modeIndex].tags;
    VerifyOrReturnError(modeTags.size() >= tags.size(), CHIP_ERROR_INVALID_ARGUMENT);
    std::copy(tags.begin(), tags.end(), modeTags.begin());
    modeTags.reduce_size(tags.size());
    return CHIP_NO_ERROR;
}

void LoggingRvcRunModeDelegate::HandleChangeToMode(uint8_t newMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    ChipLogProgress(Zcl, "LoggingRvcRunModeDelegate: ChangeToMode(%u) received.", newMode);

    if (mCluster == nullptr || mOperationalStateCluster == nullptr)
    {
        response.status = to_underlying(ModeBase::StatusCode::kSuccess);
        return;
    }

    uint8_t currentState = mOperationalStateCluster->GetCurrentOperationalState();
    uint8_t currentMode  = mCluster->GetCurrentMode();

    switch (currentState)
    {
    case to_underlying(OperationalState::OperationalStateEnum::kStopped):
    case to_underlying(RvcOperationalState::OperationalStateEnum::kDocked):
    case to_underlying(RvcOperationalState::OperationalStateEnum::kCharging):
        // We could be in the charging state with a run mode != idle.
        if (currentMode != kModeIdle && newMode != kModeIdle)
        {
            response.status = to_underlying(ModeBase::StatusCode::kInvalidInMode);
            response.statusText.SetValue("Change to the mapping or cleaning mode is only allowed from idle"_span);
            return;
        }
        LogErrorOnFailure(mOperationalStateCluster->SetOperationalState(OperationalState::OperationalStateEnum::kRunning));
        if (mServiceAreaDelegate != nullptr)
        {
            mServiceAreaDelegate->SetAttributesAtCleanStart();
        }
        response.status = to_underlying(ModeBase::StatusCode::kSuccess);
        return;

    case to_underlying(OperationalState::OperationalStateEnum::kRunning):
        if (newMode != kModeIdle)
        {
            response.status = to_underlying(ModeBase::StatusCode::kInvalidInMode);
            response.statusText.SetValue("Change to the mapping or cleaning mode is only allowed from idle"_span);
            return;
        }
        LogErrorOnFailure(mOperationalStateCluster->SetOperationalState(
            to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger)));
        if (mServiceAreaDelegate != nullptr)
        {
            mServiceAreaDelegate->UpdateProgressOnExit();
        }
        response.status = to_underlying(ModeBase::StatusCode::kSuccess);
        return;

    default:
        break;
    }

    // If we fall through, the change is not supported in the current state.
    response.status = to_underlying(ModeBase::StatusCode::kInvalidInMode);
    response.statusText.SetValue("This change is not allowed at this time"_span);
}

} // namespace chip::app::Clusters::RvcRunMode
