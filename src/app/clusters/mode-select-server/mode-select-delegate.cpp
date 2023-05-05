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
    response.status = uint8_t(ChangeToModeResponseStatus::kSuccess);
}

Status Delegate::IsSupportedMode(uint8_t modeValue) 
{ 
    for (ModeOptionStructType modeStruct : modeOptions)
    {
        if (modeStruct.mode == modeValue)
        {
            return Status::Success;
        }
    }
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Cannot find the mode %u", modeValue);
    return Status::InvalidValue;
}

Status Delegate::GetMode(uint8_t modeValue, ModeOptionStructType &modeOption)
{
    for (ModeOptionStructType modeStruct : modeOptions)
    {
        if (modeStruct.mode == modeValue)
        {
            modeOption = modeStruct;
            return Status::Success;
        }
    }
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Cannot find the mode %u", modeValue);
    return Status::InvalidValue;
}
