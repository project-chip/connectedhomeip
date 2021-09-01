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

#include <app/util/af.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CodeUtils.h>
#include <map>
#include <string>

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

bool mediaInputClusterSelectInput(uint8_t input)
{
    // TODO: Insert code here
    return true;
}
bool mediaInputClusterShowInputStatus()
{
    // TODO: Insert code here
    return true;
}
bool mediaInputClusterHideInputStatus()
{
    // TODO: Insert code here
    return true;
}
bool mediaInputClusterRenameInput(uint8_t input, std::string name)
{
    // TODO: Insert code here
    return true;
}
