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

#include <app/util/config.h>
#ifdef MATTER_DM_PLUGIN_MEDIA_INPUT_SERVER
#include "MediaInputManager.h"

using namespace std;
using namespace chip;
using namespace chip::app::Clusters::MediaInput;

MediaInputManager::MediaInputManager()
{
    struct InputData inputData1(1, chip::app::Clusters::MediaInput::InputTypeEnum::kHdmi, "HDMI 1",
                                "High-Definition Multimedia Interface");
    mInputs.push_back(inputData1);
    struct InputData inputData2(2, chip::app::Clusters::MediaInput::InputTypeEnum::kHdmi, "HDMI 2",
                                "High-Definition Multimedia Interface");
    mInputs.push_back(inputData2);
    struct InputData inputData3(3, chip::app::Clusters::MediaInput::InputTypeEnum::kHdmi, "HDMI 3",
                                "High-Definition Multimedia Interface");
    mInputs.push_back(inputData3);

    mCurrentInput = 1;
}

CHIP_ERROR MediaInputManager::HandleGetInputList(chip::app::AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (auto const & inputData : this->mInputs)
        {
            ReturnErrorOnFailure(encoder.Encode(inputData.GetEncodable()));
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
    for (auto const & inputData : mInputs)
    {
        if (inputData.index == index)
        {
            mCurrentInput = index;
            return true;
        }
    }

    return false;
}

bool MediaInputManager::HandleShowInputStatus()
{
    ChipLogProgress(Zcl, " MediaInputManager::HandleShowInputStatus()");
    for (auto const & inputData : mInputs)
    {
        ChipLogProgress(Zcl, " [%d] type=%d selected=%d name=%s desc=%s", inputData.index,
                        static_cast<uint16_t>(inputData.inputType), (mCurrentInput == inputData.index ? 1 : 0),
                        inputData.name.c_str(), inputData.description.c_str());
    }
    return true;
}

bool MediaInputManager::HandleHideInputStatus()
{
    ChipLogProgress(Zcl, " MediaInputManager::HandleHideInputStatus()");
    return true;
}

bool MediaInputManager::HandleRenameInput(const uint8_t index, const chip::CharSpan & newName)
{
    for (auto & inputData : mInputs)
    {
        if (inputData.index == index)
        {
            inputData.Rename(newName);
            return true;
        }
    }

    return false;
}
#endif // MATTER_DM_PLUGIN_MEDIA_INPUT_SERVER
