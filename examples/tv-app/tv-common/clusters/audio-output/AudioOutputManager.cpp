/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "AudioOutputManager.h"

using namespace std;
using namespace chip::app;
using namespace chip::app::Clusters::AudioOutput;

AudioOutputManager::AudioOutputManager()
{
    mCurrentOutput = 1;

    for (int i = 1; i < 4; ++i)
    {
        OutputInfoType outputInfo;
        outputInfo.outputType = chip::app::Clusters::AudioOutput::OutputTypeEnum::kHdmi;
        // note: safe only because of use of string literal
        outputInfo.name  = chip::CharSpan::fromCharString("HDMI");
        outputInfo.index = static_cast<uint8_t>(i);
        mOutputs.push_back(outputInfo);
    }
}

uint8_t AudioOutputManager::HandleGetCurrentOutput()
{
    return mCurrentOutput;
}

CHIP_ERROR AudioOutputManager::HandleGetOutputList(AttributeValueEncoder & aEncoder)
{
    // TODO: Insert code here
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
    // TODO: Insert code here
    bool audioOutputRenamed = false;

    for (OutputInfoType & output : mOutputs)
    {
        if (output.index == index)
        {
            audioOutputRenamed = true;
            memcpy(this->Data(index), name.data(), name.size());
            output.name = chip::CharSpan(this->Data(index), name.size());
        }
    }

    return audioOutputRenamed;
}

bool AudioOutputManager::HandleSelectOutput(const uint8_t & index)
{
    // TODO: Insert code here
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
