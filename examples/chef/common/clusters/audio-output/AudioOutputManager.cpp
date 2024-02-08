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
#ifdef EMBER_AF_PLUGIN_AUDIO_OUTPUT_SERVER
#include "AudioOutputManager.h"

using namespace std;
using namespace chip::app;
using namespace chip::app::Clusters::AudioOutput;
using chip::app::AttributeValueEncoder;

AudioOutputManager::AudioOutputManager()
{
    const int totalOutput = 4;

    for (uint8_t i = 1; i < totalOutput; ++i)
    {
        OutputInfoType outputInfo;
        outputInfo.outputType = chip::app::Clusters::AudioOutput::OutputTypeEnum::kHdmi;
        // note: safe only because of use of string literal
        outputInfo.name  = chip::CharSpan::fromCharString("HDMI");
        outputInfo.index = i;
        mOutputs.push_back(outputInfo);
    }
}

uint8_t AudioOutputManager::HandleGetCurrentOutput()
{
    return mCurrentOutput;
}

CHIP_ERROR AudioOutputManager::HandleGetOutputList(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (auto const & outputInfo : this->mOutputs)
        {
            ReturnErrorOnFailure(encoder.Encode(outputInfo));
        }
        return CHIP_NO_ERROR;
    });
}

bool AudioOutputManager::HandleRenameOutput(const uint8_t & index, const chip::CharSpan & name)
{
    bool audioOutputRenamed = false;

    for (OutputInfoType & output : mOutputs)
    {
        if (output.index == index)
        {
            audioOutputRenamed = true;
	    const size_t len = std::min(mBufMax, name.size());
            memcpy(this->Data(index), name.data(), len);
            output.name = chip::CharSpan(this->Data(index), len);
        }
    }

    return audioOutputRenamed;
}

bool AudioOutputManager::HandleSelectOutput(const uint8_t & index)
{
    bool audioOutputSelected = false;
    for (OutputInfoType & output : mOutputs)
    {
        if (output.index == index)
        {
            audioOutputSelected = true;
            mCurrentOutput      = index;
        }
    }

    return audioOutputSelected;
}
#endif // EMBER_AF_PLUGIN_AUDIO_OUTPUT_SERVER
