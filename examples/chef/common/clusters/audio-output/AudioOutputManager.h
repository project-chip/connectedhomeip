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
#include <string>
#include <vector>

class AudioOutputManager : public chip::app::Clusters::AudioOutput::Delegate
{
    using OutputInfoType = chip::app::Clusters::AudioOutput::Structs::OutputInfoStruct::Type;

public:
    AudioOutputManager();

    uint8_t HandleGetCurrentOutput() override;
    CHIP_ERROR HandleGetOutputList(chip::app::AttributeValueEncoder & aEncoder) override;
    bool HandleRenameOutput(const uint8_t & index, const chip::CharSpan & name) override;
    bool HandleSelectOutput(const uint8_t & index) override;

    struct OutputData
    {
        uint8_t index;
        chip::app::Clusters::AudioOutput::OutputTypeEnum outputType;
        std::string name;

        OutputData(uint8_t i, chip::app::Clusters::AudioOutput::OutputTypeEnum t, const char * n) : index(i), outputType(t), name(n)
        {}
        void Rename(const chip::CharSpan & newName) { name.assign(newName.data(), newName.size()); }
        OutputInfoType GetEncodable() const
        {
            OutputInfoType result;
            result.index      = index;
            result.outputType = outputType;
            result.name       = chip::CharSpan::fromCharString(name.c_str());
            return result;
        }
    };

protected:
    uint8_t mCurrentOutput = 1;
    std::vector<struct OutputData> mOutputs;
};
