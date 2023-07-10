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
#include <dishwasher-mode.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DishwasherMode;
using chip::Protocols::InteractionModel::Status;
template <typename T>
using List              = chip::app::DataModel::List<T>;
using ModeTagStructType = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;

CHIP_ERROR DishwasherModeInstance::AppInit()
{
    return CHIP_NO_ERROR;
}

void DishwasherModeInstance::HandleChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    response.status = static_cast<uint8_t>(ModeBase::StatusCode::kSuccess);
}

CHIP_ERROR DishwasherModeInstance::GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan & label)
{
    if (modeIndex < NumberOfModes())
    {
        if (label.size() >= kModeOptions[modeIndex].label.size())
        {
            return CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR DishwasherModeInstance::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    if (modeIndex < NumberOfModes())
    {
        value = kModeOptions[modeIndex].mode;
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR DishwasherModeInstance::GetModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> & tags)
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
