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
