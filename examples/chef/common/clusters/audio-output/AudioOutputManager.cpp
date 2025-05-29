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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/AttributeValueEncoder.h>
#include <app/util/config.h>
#include <map>

#ifdef MATTER_DM_PLUGIN_AUDIO_OUTPUT_SERVER
#include "AudioOutputManager.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::AudioOutput;
using chip::app::AttributeValueEncoder;
using chip::Protocols::InteractionModel::Status;

AudioOutputManager::AudioOutputManager(chip::EndpointId endpoint) : mEndpoint(endpoint)
{
    struct OutputData outputData1(1, chip::app::Clusters::AudioOutput::OutputTypeEnum::kHdmi, "HDMI 1");
    mOutputs.push_back(outputData1);
    struct OutputData outputData2(2, chip::app::Clusters::AudioOutput::OutputTypeEnum::kHdmi, "HDMI 2");
    mOutputs.push_back(outputData2);
    struct OutputData outputData3(3, chip::app::Clusters::AudioOutput::OutputTypeEnum::kHdmi, "HDMI 3");
    mOutputs.push_back(outputData3);
}

uint8_t AudioOutputManager::HandleGetCurrentOutput()
{
    uint8_t currentOutput = 1;
    Status status         = Attributes::CurrentOutput::Get(mEndpoint, &currentOutput);
    if (Status::Success != status)
    {
        ChipLogError(Zcl, "Unable to get CurrentOutput attribute, err:0x%x", to_underlying(status));
    }
    return currentOutput;
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
            // Sync the CurrentOutput to attribute storage while reporting changes
            Status status = Attributes::CurrentOutput::Set(mEndpoint, index);
            if (Status::Success != status)
            {
                ChipLogError(Zcl, "CurrentOutput is not stored successfully, err:0x%x", to_underlying(status));
            }
            return true;
        }
    }

    return false;
}

static std::map<chip::EndpointId, std::unique_ptr<AudioOutputManager>> gAudioOutputManagerInstance{};

void emberAfAudioOutputClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: AudioOutput::SetDefaultDelegate, endpoint=%x", endpoint);

    gAudioOutputManagerInstance[endpoint] = std::make_unique<AudioOutputManager>(endpoint);

    SetDefaultDelegate(endpoint, gAudioOutputManagerInstance[endpoint].get());
}
#endif // MATTER_DM_PLUGIN_AUDIO_OUTPUT_SERVER
