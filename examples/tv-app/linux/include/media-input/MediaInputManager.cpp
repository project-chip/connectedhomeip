/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "MediaInputManager.h"

using namespace chip;
using namespace chip::app::Clusters::MediaInput;

CHIP_ERROR MediaInputManager::HandleGetInputList(chip::app::AttributeValueEncoder & aEncoder)
{
    // TODO: Insert code here

    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        int maximumVectorSize = 2;
        for (int i = 0; i < maximumVectorSize; ++i)
        {
            chip::app::Clusters::MediaInput::Structs::InputInfo::Type inputInfo;
            inputInfo.description = chip::CharSpan::fromCharString("exampleDescription");
            inputInfo.name        = chip::CharSpan::fromCharString("exampleName");
            inputInfo.inputType   = chip::app::Clusters::MediaInput::InputTypeEnum::kHdmi;
            inputInfo.index       = static_cast<uint8_t>(1 + i);

            ReturnErrorOnFailure(encoder.Encode(inputInfo));
        }

        return CHIP_NO_ERROR;
    });
}

uint8_t MediaInputManager::HandleGetCurrentInput()
{
    return 0;
}

bool MediaInputManager::HandleSelectInput(const uint8_t index)
{
    // TODO: Insert code here
    return true;
}

bool MediaInputManager::HandleShowInputStatus()
{
    // TODO: Insert code here
    return true;
}

bool MediaInputManager::HandleHideInputStatus()
{
    // TODO: Insert code here
    return true;
}

bool MediaInputManager::HandleRenameInput(const uint8_t index, const chip::CharSpan & name)
{
    // TODO: Insert code here
    return true;
}
