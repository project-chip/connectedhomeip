/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "MediaInputManager.h"
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/util/af.h>
#include <lib/core/CHIPSafeCasts.h>
#include <map>
#include <string>
#include <support/CodeUtils.h>

CHIP_ERROR MediaInputManager::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // TODO: Store feature map once it is supported
    std::map<std::string, bool> featureMap;
    featureMap["NU"] = true;
    SuccessOrExit(err);
exit:
    return err;
}

bool MediaInputManager::proxySelectInputRequest(uint8_t input)
{
    // TODO: Insert code here
    return true;
}

bool MediaInputManager::proxyShowInputStatusRequest()
{
    // TODO: Insert code here
    return true;
}

bool MediaInputManager::proxyHideInputStatusRequest()
{
    // TODO: Insert code here
    return true;
}

bool MediaInputManager::proxyRenameInputRequest(uint8_t input, std::string name)
{
    // TODO: Insert code here
    return true;
}

std::vector<EmberAfMediaInputInfo> MediaInputManager::proxyGetInputList()
{
    // TODO: Insert code here
    std::vector<EmberAfMediaInputInfo> mediaInputList;
    int maximumVectorSize = 2;
    char description[]    = "exampleDescription";
    char name[]           = "exampleName";

    for (int i = 0; i < maximumVectorSize; ++i)
    {
        EmberAfMediaInputInfo mediaInput;
        mediaInput.description = chip::ByteSpan(chip::Uint8::from_char(description), sizeof(description));
        mediaInput.name        = chip::ByteSpan(chip::Uint8::from_char(name), sizeof(name));
        mediaInput.inputType   = EMBER_ZCL_MEDIA_INPUT_TYPE_HDMI;
        mediaInput.index       = static_cast<uint8_t>(1 + i);
        mediaInputList.push_back(mediaInput);
    }

    return mediaInputList;
}

static void storeCurrentInput(chip::EndpointId endpoint, uint8_t currentInput)
{
    EmberAfStatus status =
        emberAfWriteServerAttribute(endpoint, ZCL_MEDIA_INPUT_CLUSTER_ID, ZCL_MEDIA_INPUT_CURRENT_INPUT_ATTRIBUTE_ID,
                                    (uint8_t *) &currentInput, ZCL_INT8U_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to store media playback attribute.");
    }
}

bool emberAfMediaInputClusterSelectInputCallback(uint8_t input)
{
    bool success         = MediaInputManager().proxySelectInputRequest(input);
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    if (success)
    {
        storeCurrentInput(emberAfCurrentEndpoint(), input);
    }
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfMediaInputClusterShowInputStatusCallback()
{
    bool success         = MediaInputManager().proxyShowInputStatusRequest();
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfMediaInputClusterHideInputStatusCallback()
{
    bool success         = MediaInputManager().proxyHideInputStatusRequest();
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfMediaInputClusterRenameInputCallback(uint8_t input, char * name)
{
    std::string nameString(name);
    bool success         = MediaInputManager().proxyRenameInputRequest(input, nameString);
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
