/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/AttributeAccessInterface.h>
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
