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

#include "AppKeypadInputManager.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <lib/support/CHIPJNIError.h>

using namespace chip;
using namespace chip::app::Clusters::KeypadInput;

void AppKeypadInputManager::HandleSendKey(CommandResponseHelper<SendKeyResponseType> & helper, const CecKeyCode & keyCode)
{
    Commands::SendKeyResponse::Type response;

    ChipLogProgress(Zcl, "Received keypadInputClusterSendKey: %c", to_underlying(keyCode));

    // Insert code here
    if (to_underlying(keyCode) == 116)
    {
        response.status = chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kUnsupportedKey;
    }
    else
    {
        response.status = chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kSuccess;
    }
    helper.Success(response);
}

uint32_t AppKeypadInputManager::GetFeatureMap(chip::EndpointId endpoint)
{
    if (endpoint >= EMBER_AF_CONTENT_LAUNCH_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        return mDynamicEndpointFeatureMap;
    }

    uint32_t featureMap = 0;
    Attributes::FeatureMap::Get(endpoint, &featureMap);
    return featureMap;
}
