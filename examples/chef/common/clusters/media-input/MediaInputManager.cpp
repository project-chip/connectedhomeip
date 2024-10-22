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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/config.h>
#include <map>

#ifdef MATTER_DM_PLUGIN_MEDIA_INPUT_SERVER
#include "MediaInputManager.h"

using namespace chip;
using namespace chip::app::Clusters::MediaInput;
using Protocols::InteractionModel::Status;

MediaInputManager::MediaInputManager(chip::EndpointId endpoint) : mEndpoint(endpoint)
{
    struct InputData inputData1(1, InputTypeEnum::kHdmi, "HDMI 1", "High-Definition Multimedia Interface");
    mInputs.push_back(inputData1);
    struct InputData inputData2(2, InputTypeEnum::kHdmi, "HDMI 2", "High-Definition Multimedia Interface");
    mInputs.push_back(inputData2);
    struct InputData inputData3(3, InputTypeEnum::kHdmi, "HDMI 3", "High-Definition Multimedia Interface");
    mInputs.push_back(inputData3);
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
    uint8_t currentInput = 1;
    Status status        = Attributes::CurrentInput::Get(mEndpoint, &currentInput);
    if (Status::Success != status)
    {
        ChipLogError(Zcl, "Unable to get CurrentInput attribute, err:0x%x", to_underlying(status));
    }
    return currentInput;
}

bool MediaInputManager::HandleSelectInput(const uint8_t index)
{
    if (HandleGetCurrentInput() == index)
    {
        ChipLogProgress(Zcl, "CurrentInput is same as new value: %u", index);
        return true;
    }
    for (auto const & inputData : mInputs)
    {
        if (inputData.index == index)
        {
            // Sync the CurrentInput to attribute storage while reporting changes
            Status status = Attributes::CurrentInput::Set(mEndpoint, index);
            if (Status::Success != status)
            {
                ChipLogError(Zcl, "CurrentInput is not stored successfully, err:0x%x", to_underlying(status));
            }
            return true;
        }
    }

    return false;
}

bool MediaInputManager::HandleShowInputStatus()
{
    uint8_t currentInput = HandleGetCurrentInput();
    ChipLogProgress(Zcl, " MediaInputManager::HandleShowInputStatus()");
    for (auto const & inputData : mInputs)
    {
        ChipLogProgress(Zcl, " [%d] type=%d selected=%d name=%s desc=%s", inputData.index,
                        static_cast<uint16_t>(inputData.inputType), (currentInput == inputData.index ? 1 : 0),
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

static std::map<chip::EndpointId, std::unique_ptr<MediaInputManager>> gMediaInputManagerInstance{};

void emberAfMediaInputClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: MediaInput::SetDefaultDelegate, endpoint=%x", endpoint);

    gMediaInputManagerInstance[endpoint] = std::make_unique<MediaInputManager>(endpoint);

    SetDefaultDelegate(endpoint, gMediaInputManagerInstance[endpoint].get());
}
#endif // MATTER_DM_PLUGIN_MEDIA_INPUT_SERVER
