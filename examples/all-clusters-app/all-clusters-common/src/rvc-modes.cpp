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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <rvc-modes.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::RvcRunMode;
using namespace chip::app::Clusters::RvcCleanMode;
using chip::Protocols::InteractionModel::Status;
template <typename T>
using List              = chip::app::DataModel::List<T>;
using ModeTagStructType = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;

CHIP_ERROR RvcRunModeInstance::AppInit()
{
    return CHIP_NO_ERROR;
}

void RvcRunModeInstance::HandleChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    uint8_t currentMode = GetCurrentMode();

    // Our business logic states that we can only switch into the mapping state from the idle state.
    if (NewMode == RvcRunMode::ModeMapping && currentMode != RvcRunMode::ModeIdle)
    {
        response.status = to_underlying(ModeBase::StatusCode::kGenericFailure);
        response.statusText.SetValue(chip::CharSpan("Change to the mapping mode is only allowed from idle", 52));
    }

    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
}

CHIP_ERROR RvcRunModeInstance::GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan & label)
{
    if (modeIndex < NumberOfModes())
    {
        return CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR RvcRunModeInstance::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    if (modeIndex < NumberOfModes())
    {
        value = kModeOptions[modeIndex].mode;
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR RvcRunModeInstance::GetModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> & tags)
{
    if (modeIndex < NumberOfModes())
    {
        if (tags.size() >= kModeOptions[modeIndex].modeTags.size())
        {
            std::copy(kModeOptions[modeIndex].modeTags.begin(), kModeOptions[modeIndex].modeTags.end(), tags.begin());
            tags.reduce_size(kModeOptions[modeIndex].modeTags.size());

            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR RvcCleanModeInstance::AppInit()
{
    return CHIP_NO_ERROR;
}

void RvcCleanModeInstance::HandleChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    uint8_t rvcRunCurrentMode = GetCurrentMode();

    if (rvcRunCurrentMode == RvcRunMode::ModeCleaning)
    {
        response.status = to_underlying(RvcCleanMode::StatusCode::kCleaningInProgress);
        response.statusText.SetValue(chip::CharSpan("Cannot change the cleaning mode during a clean", 60));
    }

    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
}

CHIP_ERROR RvcCleanModeInstance::GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan & label)
{
    if (modeIndex < NumberOfModes())
    {
        return CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR RvcCleanModeInstance::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    if (modeIndex < NumberOfModes())
    {
        value = kModeOptions[modeIndex].mode;
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR RvcCleanModeInstance::GetModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> & tags)
{
    if (modeIndex < NumberOfModes())
    {
        if (tags.size() >= kModeOptions[modeIndex].modeTags.size())
        {
            std::copy(kModeOptions[modeIndex].modeTags.begin(), kModeOptions[modeIndex].modeTags.end(), tags.begin());
            tags.reduce_size(kModeOptions[modeIndex].modeTags.size());

            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_ERROR_NOT_FOUND;
}
