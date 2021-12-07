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

#include "AudioOutputManager.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/af.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CodeUtils.h>

#include <map>
#include <string>

using namespace std;

CHIP_ERROR AudioOutputManager::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // TODO: Store feature map once it is supported
    map<string, bool> featureMap;
    featureMap["NU"] = true;

    return err;
}

CHIP_ERROR AudioOutputManager::proxyGetListOfAudioOutputInfo(chip::app::AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        // TODO: Insert code here
        int maximumVectorSize = 3;
        char name[]           = "exampleName";

        for (int i = 0; i < maximumVectorSize; ++i)
        {
            chip::app::Clusters::AudioOutput::Structs::AudioOutputInfo::Type audioOutputInfo;
            audioOutputInfo.outputType = EMBER_ZCL_AUDIO_OUTPUT_TYPE_HDMI;
            audioOutputInfo.name       = chip::CharSpan(name, sizeof(name) - 1);
            audioOutputInfo.index      = static_cast<uint8_t>(1 + i);
            ReturnErrorOnFailure(encoder.Encode(audioOutputInfo));
        }
        return CHIP_NO_ERROR;
    });
}

bool audioOutputClusterSelectOutput(uint8_t index)
{
    // TODO: Insert code here
    return true;
}
bool audioOutputClusterRenameOutput(uint8_t index, const chip::CharSpan & name)
{
    // TODO: Insert code here
    return true;
}
