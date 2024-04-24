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
#include <vector>

using InputInfoType = chip::app::Clusters::MediaInput::Structs::InputInfoStruct::Type;

class MediaInputManager : public chip::app::Clusters::MediaInput::Delegate
{
public:
    MediaInputManager();

    CHIP_ERROR HandleGetInputList(chip::app::AttributeValueEncoder & aEncoder) override;
    uint8_t HandleGetCurrentInput() override;
    bool HandleSelectInput(const uint8_t index) override;
    bool HandleShowInputStatus() override;
    bool HandleHideInputStatus() override;
    bool HandleRenameInput(const uint8_t index, const chip::CharSpan & name) override;
    char * Data(uint8_t index) { return mCharDataBuffer[index]; }

protected:
    uint8_t mCurrentInput;
    std::vector<chip::app::Clusters::MediaInput::Structs::InputInfoStruct::Type> mInputs;
    // Magic numbers are here on purpose, please allocate memory
    char mCharDataBuffer[10][32];
};
