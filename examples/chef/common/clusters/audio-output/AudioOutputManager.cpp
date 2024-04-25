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

#include <app/util/config.h>
#ifdef MATTER_DM_PLUGIN_AUDIO_OUTPUT_SERVER
#include "AudioOutputManager.h"

using namespace std;
using namespace chip::app;
using namespace chip::app::Clusters::AudioOutput;
using chip::app::AttributeValueEncoder;

AudioOutputManager::AudioOutputManager()
{
    struct OutputData outputData1(1, chip::app::Clusters::AudioOutput::OutputTypeEnum::kHdmi, "HDMI 1");
    mOutputs.push_back(outputData1);
    struct OutputData outputData2(2, chip::app::Clusters::AudioOutput::OutputTypeEnum::kHdmi, "HDMI 2");
    mOutputs.push_back(outputData2);
    struct OutputData outputData3(3, chip::app::Clusters::AudioOutput::OutputTypeEnum::kHdmi, "HDMI 3");
    mOutputs.push_back(outputData3);

    mCurrentOutput = 1;
}

uint8_t AudioOutputManager::HandleGetCurrentOutput()
{
    return mCurrentOutput;
}

CHIP_ERROR AudioOutputManager::HandleGetOutputList(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (auto const & outputData : mOutputs)
        {
            ReturnErrorOnFailure(encoder.Encode(outputData.GetEncodable()));
        }
        return CHIP_NO_ERROR;
    });
}

bool AudioOutputManager::HandleRenameOutput(const uint8_t & index, const chip::CharSpan & newName)
{
    for (auto & outputData : mOutputs)
    {
        if (outputData.index == index)
        {
            outputData.Rename(newName);
            return true;
        }
    }

    return false;
}

bool AudioOutputManager::HandleSelectOutput(const uint8_t & index)
{
    for (auto & outputData : mOutputs)
    {
        if (outputData.index == index)
        {
            mCurrentOutput = index;
            return true;
        }
    }

    return false;
}
#endif // MATTER_DM_PLUGIN_AUDIO_OUTPUT_SERVER
