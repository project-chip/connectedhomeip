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
#include <rvc-mode-delegates.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::RvcRunMode;
using namespace chip::app::Clusters::RvcCleanMode;
using chip::Protocols::InteractionModel::Status;

//-- Mode Select delegate functions

CHIP_ERROR RvcRunModeDelegate::Init()
{
    return CHIP_NO_ERROR;
}

void RvcRunModeDelegate::HandleChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type &response)
{
    uint8_t currentMode;
    EmberAfStatus status = RvcRunMode::Attributes::CurrentMode::Get(0x1, &currentMode);

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        response.status = static_cast<uint8_t>(ModeBase::StatusCode::kGenericFailure);
        response.statusText.SetValue(chip::CharSpan("Could not get the current mode", 30));
        return;
    }

    // Our business logic states that we can only switch into the mapping state from the idle state.
    if (NewMode == RvcRunMode::ModeMapping && currentMode != RvcRunMode::ModeIdle)
    {
        response.status = static_cast<uint8_t>(ModeBase::StatusCode::kGenericFailure);
        response.statusText.SetValue(chip::CharSpan("Change to the mapping state is only allowed from idle", 53));
    }

    response.status = static_cast<uint8_t>(ModeBase::StatusCode::kSuccess);
}

CHIP_ERROR RvcRunModeDelegate::getModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan &label)
{
    if (modeIndex < NumberOfModes()) {
        if (label.size() >= modeOptions[modeIndex].label.size())
        {
            CopyCharSpanToMutableCharSpan(modeOptions[modeIndex].label, label);
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR RvcRunModeDelegate::getModeValueByIndex(uint8_t modeIndex, uint8_t &value)
{
    if (modeIndex < NumberOfModes()) {
        value = modeOptions[modeIndex].mode;
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR RvcRunModeDelegate::getModeTagsByIndex(uint8_t modeIndex, List<SemanticTagStructType> &tags)
{
    if (modeIndex < NumberOfModes()) {
        if (tags.size() >= modeOptions[modeIndex].modeTags.size())
        {
            std::copy(modeOptions[modeIndex].modeTags.begin(), modeOptions[modeIndex].modeTags.end(), tags.begin());
            tags.reduce_size(modeOptions[modeIndex].modeTags.size());

            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_ERROR_NOT_FOUND;
}


CHIP_ERROR RvcCleanModeDelegate::Init()
{
    return CHIP_NO_ERROR;
}

void RvcCleanModeDelegate::HandleChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type &response)
{
    uint8_t rvcRunCurrentMode;
    EmberAfStatus status = RvcRunMode::Attributes::CurrentMode::Get(0x1, &rvcRunCurrentMode);

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        response.status = static_cast<uint8_t>(ModeBase::StatusCode::kGenericFailure);
        response.statusText.SetValue(chip::CharSpan("Could not get the current mode", 30));
        return;
    }

    if (rvcRunCurrentMode == RvcRunMode::ModeCleaning)
    {
        response.status = static_cast<uint8_t>(RvcCleanMode::StatusCode::kCleaningInProgress);
        response.statusText.SetValue(chip::CharSpan("Cannot change the cleaning mode during a clean", 60));
    }

    response.status = static_cast<uint8_t>(ModeBase::StatusCode::kSuccess);
}

CHIP_ERROR RvcCleanModeDelegate::getModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan &label)
{
    if (modeIndex < NumberOfModes()) {
        if (label.size() >= modeOptions[modeIndex].label.size())
        {
            CopyCharSpanToMutableCharSpan(modeOptions[modeIndex].label, label);
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR RvcCleanModeDelegate::getModeValueByIndex(uint8_t modeIndex, uint8_t &value)
{
    if (modeIndex < NumberOfModes()) {
        value = modeOptions[modeIndex].mode;
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR RvcCleanModeDelegate::getModeTagsByIndex(uint8_t modeIndex, List<SemanticTagStructType> &tags)
{
    if (modeIndex < NumberOfModes()) {
        if (tags.size() >= modeOptions[modeIndex].modeTags.size())
        {
            std::copy(modeOptions[modeIndex].modeTags.begin(), modeOptions[modeIndex].modeTags.end(), tags.begin());
            tags.reduce_size(modeOptions[modeIndex].modeTags.size());

            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_ERROR_NOT_FOUND;
}
