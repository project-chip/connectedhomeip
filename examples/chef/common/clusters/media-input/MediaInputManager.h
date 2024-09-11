/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#pragma once

#include <app/AttributeValueEncoder.h>
#include <app/clusters/media-input-server/media-input-server.h>

#include <string>
#include <vector>

class MediaInputManager : public chip::app::Clusters::MediaInput::Delegate
{
    using InputInfoType = chip::app::Clusters::MediaInput::Structs::InputInfoStruct::Type;

public:
    MediaInputManager(chip::EndpointId endpoint);

    CHIP_ERROR HandleGetInputList(chip::app::AttributeValueEncoder & aEncoder) override;
    uint8_t HandleGetCurrentInput() override;
    bool HandleSelectInput(const uint8_t index) override;
    bool HandleShowInputStatus() override;
    bool HandleHideInputStatus() override;
    bool HandleRenameInput(const uint8_t index, const chip::CharSpan & name) override;

    struct InputData
    {
        uint8_t index;
        chip::app::Clusters::MediaInput::InputTypeEnum inputType;
        std::string name;
        std::string description;

        InputData(uint8_t i, chip::app::Clusters::MediaInput::InputTypeEnum t, const char * n, const char * d) :
            index(i), inputType(t), name(n), description(d)
        {}

        void Rename(const chip::CharSpan & newName) { name.assign(newName.data(), newName.size()); }

        InputInfoType GetEncodable() const
        {
            InputInfoType result;
            result.index       = index;
            result.inputType   = inputType;
            result.name        = chip::CharSpan::fromCharString(name.c_str());
            result.description = chip::CharSpan::fromCharString(description.c_str());
            return result;
        }
    };

protected:
    chip::EndpointId mEndpoint;
    std::vector<InputData> mInputs;

private:
};
