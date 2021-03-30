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

#include "gen/attribute-id.h"
#include "gen/attribute-type.h"
#include "gen/cluster-id.h"
#include "gen/command-id.h"

#include <app/util/af.h>
#include <app/util/basic-types.h>

#include <map>
#include <string>

using namespace std;

CHIP_ERROR AudioOutputManager::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // TODO: Store feature map once it is supported
    map<string, bool> featureMap;
    featureMap["NU"] = true;

    // TODO: when list is not supported enable
    // list<EmberAfAudioOutputInfo> listOfAudioOutputs = AudioOutputManager().proxyGetListOfAudioOutputInfo();
    // EmberAfStatus status = emberAfWriteServerAttribute(endpoint, ZCL_AUDIO_OUTPUT_CLUSTER_ID, ZCL_AUDIO_OUTPUT_LIST_ATTRIBUTE_ID,
    //                                                    (uint8_t *) &listOfAudioOutputs, ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
    // if (status != EMBER_ZCL_STATUS_SUCCESS)
    // {
    //     emberAfAudioOutputClusterPrintln("Failed to write Output List attribute: 0x%X", status);
    // }
    return err;
}

list<EmberAfAudioOutputInfo> AudioOutputManager::proxyGetListOfAudioOutputInfo()
{
    // TODO: Insert code here
    list<EmberAfAudioOutputInfo> listOfAudioOutputInfos;
    EmberAfAudioOutputInfo audioOutputInfo = {};
    // audioOutputInfo.Name                   = "";
    audioOutputInfo.outputType = EMBER_ZCL_AUDIO_OUTPUT_TYPE_HDMI;
    audioOutputInfo.index      = 1;

    listOfAudioOutputInfos.push_back(audioOutputInfo);
    return listOfAudioOutputInfos;
}

bool AudioOutputManager::proxySelectOutputRequest(uint8_t index)
{
    // TODO: Insert code here
    return true;
}
bool AudioOutputManager::proxyRenameOutputRequest(uint8_t index, uint8_t * name)
{
    // TODO: Insert code here
    return true;
}

bool emberAfAudioOutputClusterRenameOutputCallback(unsigned char index, uint8_t * name)
{
    bool success         = AudioOutputManager().proxyRenameOutputRequest(index, name);
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfAudioOutputClusterSelectOutputCallback(unsigned char index)
{
    bool success         = AudioOutputManager().proxySelectOutputRequest(index);
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
