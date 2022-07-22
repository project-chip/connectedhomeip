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

using namespace std;
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
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (auto const & inputInfo : this->mInputs)
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

bool MediaInputManager::HandleSelectInput(const uint8_t index)
{
    // TODO: Insert code here
    bool mediaInputSelected = false;
    for (InputInfoType & input : mInputs)
    {
        if (input.index == index)
        {
            mediaInputSelected = true;
            mCurrentInput      = index;
        }
    }

    return mediaInputSelected;
}

bool MediaInputManager::HandleShowInputStatus()
{
    ChipLogProgress(Zcl, " MediaInputManager::HandleShowInputStatus()");
    for (auto const & inputInfo : this->mInputs)
    {
        string name(inputInfo.name.data(), inputInfo.name.size());
        string desc(inputInfo.description.data(), inputInfo.description.size());
        ChipLogProgress(Zcl, " [%d] type=%d selected=%d name=%s desc=%s", inputInfo.index,
                        static_cast<uint16_t>(inputInfo.inputType), (mCurrentInput == inputInfo.index ? 1 : 0), name.c_str(),
                        desc.c_str());
    }
    return true;
}

bool MediaInputManager::HandleHideInputStatus()
{
    ChipLogProgress(Zcl, " MediaInputManager::HandleHideInputStatus()");
    return true;
}

bool MediaInputManager::HandleRenameInput(const uint8_t index, const chip::CharSpan & name)
{
    // TODO: Insert code here
    bool mediaInputRenamed = false;

    for (InputInfoType & input : mInputs)
    {
        if (input.index == index)
        {
            mediaInputRenamed = true;
            memcpy(this->Data(index), name.data(), name.size());
            input.name = chip::CharSpan(this->Data(index), name.size());
        }
    }

    return mediaInputRenamed;
}
