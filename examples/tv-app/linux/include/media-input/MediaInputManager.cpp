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

MediaInputManager::MediaInputManager()
{
    mCurrentInput = 1;

    for (int i = 1; i < 3; ++i)
    {
        InputInfoType inputInfo;
        inputInfo.description = chip::CharSpan::fromCharString("High-Definition Multimedia Interface");
        inputInfo.name        = chip::CharSpan::fromCharString("HDMI");
        inputInfo.inputType   = chip::app::Clusters::MediaInput::InputTypeEnum::kHdmi;
        inputInfo.index       = static_cast<uint8_t>(i);
        mInputs.push_back(inputInfo);
    }
}

CHIP_ERROR MediaInputManager::HandleGetInputList(chip::app::AttributeValueEncoder & aEncoder)
{
    // TODO: Insert code here
    std::vector<InputInfoType> inputs = mInputs;
    return aEncoder.EncodeList([inputs](const auto & encoder) -> CHIP_ERROR {
        for (auto const & inputInfo : inputs)
        {
            ReturnErrorOnFailure(encoder.Encode(inputInfo));
        }
        return CHIP_NO_ERROR;
    });
}

uint8_t MediaInputManager::HandleGetCurrentInput()
{
    return mCurrentInput;
}

bool isMediaInputIndexInRange(const uint8_t index, std::vector<InputInfoType> inputs)
{
    return index > 0 && index <= inputs.size();
}

bool MediaInputManager::HandleSelectInput(const uint8_t index)
{
    // TODO: Insert code here
    if (isMediaInputIndexInRange(index, mInputs))
    {
        mCurrentInput = index;
        return true;
    }
    else
    {
        return false;
    }
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
    if (isMediaInputIndexInRange(index, mInputs))
    {
        uint16_t counter = 0;
        for (const InputInfoType & input : mInputs)
        {
            if (input.index == index)
            {
                mInputs[counter].name = name;
            }
            counter++;
        }
        return true;
    }
    else
    {
        return false;
    }
}
