/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/clusters/audio-output-server/audio-output-server.h>
#include <vector>

using chip::app::AttributeValueEncoder;
using AudioOutputDelegate = chip::app::Clusters::AudioOutput::Delegate;
using OutputInfoType      = chip::app::Clusters::AudioOutput::Structs::OutputInfoStruct::Type;

class AudioOutputManager : public AudioOutputDelegate
{
public:
    AudioOutputManager();

    uint8_t HandleGetCurrentOutput() override;
    CHIP_ERROR HandleGetOutputList(AttributeValueEncoder & aEncoder) override;
    bool HandleRenameOutput(const uint8_t & index, const chip::CharSpan & name) override;
    bool HandleSelectOutput(const uint8_t & index) override;
    char * Data(uint8_t index) { return mCharDataBuffer[index]; }

protected:
    uint8_t mCurrentOutput;
    std::vector<OutputInfoType> mOutputs;
    // Magic numbers are here on purpose, please allocate memory
    char mCharDataBuffer[10][32];
};
