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

#include <app/clusters/mode-select-server/mode-select-server.h>
#include "mode-select-delegate.h"

using namespace std;
using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ModeSelect;

using ModeOptionStructType = Structs::ModeOptionStruct::Type;

Status Delegate::HandleChangeToMode(uint8_t mode)
{
    return Status::Success;
}

void Delegate::HandleChangeToModeWitheStatus(uint8_t mode, ModeSelect::Commands::ChangeToModeResponse::Type &response)
{
    response.status = to_underlying(ChangeToModeResponseStatus::kSuccess);
}

uint8_t Delegate::NumberOfModes()
{
    return 0;
}

CHIP_ERROR Delegate::getModeLabelByIndex(uint8_t modeIndex, MutableCharSpan &label)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR Delegate::getModeValueByIndex(uint8_t modeIndex, uint8_t &value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR Delegate::getModeTagsByIndex(uint8_t modeIndex, List<SemanticTagStructType> &tags)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

bool Delegate::IsSupportedMode(uint8_t modeValue)
{
    for (uint8_t i = 0; i < NumberOfModes(); i++) {
        uint8_t value;
        auto err = getModeValueByIndex(i, value);
        if (err == CHIP_NO_ERROR) {
            if (value == modeValue) {
                return true;
            }
        } else {
            break;
        }
    }
    ChipLogDetail(Zcl, "Cannot find the mode %u", modeValue);
    return false;
}
