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
#include <mode-select-delegates.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ModeSelect;
using chip::Protocols::InteractionModel::Status;

//-- Mode Select delegate functions

CHIP_ERROR ModeSelectDelegate::Init()
{
    return CHIP_NO_ERROR;
}

Status ModeSelectDelegate::HandleChangeToMode(uint8_t mode)
{
    return Status::Success;
}

void ModeSelectDelegate::HandleChangeToModeWitheStatus(uint8_t mode, Commands::ChangeToModeResponse::Type &response)
{
    uint8_t currentMode;
    EmberAfStatus status = Attributes::CurrentMode::Get(0x1, &currentMode);

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        response.status = static_cast<uint8_t>(ChangeToModeResponseStatus::kGenericFailure);
        response.statusText.SetValue(chip::CharSpan("Could not get the current mode", 30));
        return;
    }

    // We add our business logic that doesn't allow a transition from mode 0 to 7. If this is attempted, a descriptive
    // response is sent.
    if (currentMode == ModeBlack && mode == ModeEspresso)
    {
        response.status = static_cast<uint8_t>(ChangeToModeResponseStatus::kGenericFailure);
        response.statusText.SetValue(chip::CharSpan("Cannot change to Espresso from mode Black", 41));
        return;
    }
}

CHIP_ERROR ModeSelectDelegate::getModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan &label)
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

CHIP_ERROR ModeSelectDelegate::getModeValueByIndex(uint8_t modeIndex, uint8_t &value)
{
    if (modeIndex < NumberOfModes()) {
        value = modeOptions[modeIndex].mode;
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR ModeSelectDelegate::getModeTagsByIndex(uint8_t modeIndex, List<SemanticTagStructType> &tags)
{
    if (modeIndex < NumberOfModes()) {
        if (tags.size() >= modeOptions[modeIndex].semanticTags.size())
        {
            std::copy(modeOptions[modeIndex].semanticTags.begin(), modeOptions[modeIndex].semanticTags.end(), tags.begin());
            tags.reduce_size(modeOptions[modeIndex].semanticTags.size());

            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_ERROR_NOT_FOUND;
}


////-- RVC Run delegate functions
//CHIP_ERROR RvcRunDelegate::Init()
//{
//    return CHIP_NO_ERROR;
//}
//
//Status RvcRunDelegate::HandleChangeToMode(uint8_t mode)
//{
//    return Status::UnsupportedCommand;
//}
//
//void RvcRunDelegate::HandleChangeToModeWitheStatus(uint8_t mode, Commands::ChangeToModeResponse::Type &response)
//{
//    uint8_t currentMode;
//    EmberAfStatus status = RvcRun::Attributes::CurrentMode::Get(0x1, &currentMode);
//
//    if (status != EMBER_ZCL_STATUS_SUCCESS)
//    {
//        response.status = static_cast<uint8_t>(ChangeToModeResponseStatus::kGenericFailure);
//        response.statusText.SetValue(chip::CharSpan("Could not get the current mode", 30));
//        return;
//    }
//
//    // Our business logic states that we can only switch into the mapping state from the idle state.
//    if (mode == RvcRun::ModeMapping && currentMode != RvcRun::ModeIdle)
//    {
//        response.status = static_cast<uint8_t>(ChangeToModeResponseStatus::kGenericFailure);
//        response.statusText.SetValue(chip::CharSpan("Change to the mapping state is only allowed from idle", 53));
//    }
//
//    response.status = static_cast<uint8_t>(ChangeToModeResponseStatus::kSuccess);
//}
//
////-- RVC Clean delegate functions
//CHIP_ERROR RvcCleanDelegate::Init()
//{
//    return CHIP_NO_ERROR;
//}
//
//Status RvcCleanDelegate::HandleChangeToMode(uint8_t mode)
//{
//    return Status::UnsupportedCommand;
//}
//
//void RvcCleanDelegate::HandleChangeToModeWitheStatus(uint8_t mode, Commands::ChangeToModeResponse::Type &response)
//{
//    uint8_t rvcRunCurrentMode;
//    EmberAfStatus status = RvcRun::Attributes::CurrentMode::Get(0x1, &rvcRunCurrentMode);
//
//    if (status != EMBER_ZCL_STATUS_SUCCESS)
//    {
//        response.status = static_cast<uint8_t>(ChangeToModeResponseStatus::kGenericFailure);
//        response.statusText.SetValue(chip::CharSpan("Could not get the current mode", 30));
//        return;
//    }
//
//    if (rvcRunCurrentMode == RvcRun::ModeCleaning)
//    {
//        response.status = static_cast<uint8_t>(RvcClean::ChangeToModeResponseStatus::kCleaningInProgress);
//        response.statusText.SetValue(chip::CharSpan("Cannot change the cleaning mode during a clean", 60));
//    }
//
//    response.status = static_cast<uint8_t>(ChangeToModeResponseStatus::kSuccess);
//}
//
////-- Dishwasher Control delegate functions
//CHIP_ERROR DishwasherControlDelegate::Init()
//{
//    return CHIP_NO_ERROR;
//}
//
//Status DishwasherControlDelegate::HandleChangeToMode(uint8_t mode)
//{
//    return Status::Success;
//}
//
//void DishwasherControlDelegate::HandleChangeToModeWitheStatus(uint8_t mode, Commands::ChangeToModeResponse::Type &response)
//{
//    response.status = static_cast<uint8_t>(ChangeToModeResponseStatus::kSuccess);
//}
//
////-- Laundry Washer delegate functions
//CHIP_ERROR LaundryWasherDelegate::Init()
//{
//    return CHIP_NO_ERROR;
//}
//
//Status LaundryWasherDelegate::HandleChangeToMode(uint8_t mode)
//{
//    return Status::Success;
//}
//
//void LaundryWasherDelegate::HandleChangeToModeWitheStatus(uint8_t mode, Commands::ChangeToModeResponse::Type &response)
//{
//    response.status = static_cast<uint8_t>(ChangeToModeResponseStatus::kSuccess);
//}
//
////-- Refrigerator And Temperature Controlled Cabinet delegate functions
//CHIP_ERROR TccDelegate::Init()
//{
//    return CHIP_NO_ERROR;
//}
//
//Status TccDelegate::HandleChangeToMode(uint8_t mode)
//{
//    return Status::Success;
//}
//
//void TccDelegate::HandleChangeToModeWitheStatus(uint8_t mode, Commands::ChangeToModeResponse::Type &response)
//{
//    response.status = static_cast<uint8_t>(ChangeToModeResponseStatus::kSuccess);
//}
