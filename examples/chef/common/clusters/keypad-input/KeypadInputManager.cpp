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
#ifdef MATTER_DM_PLUGIN_KEYPAD_INPUT_SERVER
#include "KeypadInputManager.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/config.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::KeypadInput;

using chip::app::CommandResponseHelper;

void KeypadInputManager::HandleSendKey(CommandResponseHelper<SendKeyResponseType> & helper, const CecKeyCodeType & keycCode)
{
    Commands::SendKeyResponse::Type response;

    switch (keycCode)
    {
    case CecKeyCodeType::kUp:
    case CecKeyCodeType::kDown:
    case CecKeyCodeType::kLeft:
    case CecKeyCodeType::kRight:
    case CecKeyCodeType::kSelect:
    case CecKeyCodeType::kBackward:
    case CecKeyCodeType::kExit:
    case CecKeyCodeType::kRootMenu:
    case CecKeyCodeType::kSetupMenu:
    case CecKeyCodeType::kEnter:
    case CecKeyCodeType::kNumber0OrNumber10:
    case CecKeyCodeType::kNumbers1:
    case CecKeyCodeType::kNumbers2:
    case CecKeyCodeType::kNumbers3:
    case CecKeyCodeType::kNumbers4:
    case CecKeyCodeType::kNumbers5:
    case CecKeyCodeType::kNumbers6:
    case CecKeyCodeType::kNumbers7:
    case CecKeyCodeType::kNumbers8:
    case CecKeyCodeType::kNumbers9:
        response.status = chip::app::Clusters::KeypadInput::StatusEnum::kSuccess;
        break;
    default:
        response.status = chip::app::Clusters::KeypadInput::StatusEnum::kUnsupportedKey;
        break;
    }

    helper.Success(response);
}

uint32_t KeypadInputManager::GetFeatureMap(chip::EndpointId endpoint)
{
    if (endpoint >= MATTER_DM_KEYPAD_INPUT_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        return mDynamicEndpointFeatureMap;
    }

    uint32_t featureMap = 0;
    Attributes::FeatureMap::Get(endpoint, &featureMap);
    return featureMap;
}
#endif // MATTER_DM_PLUGIN_KEYPAD_INPUT_SERVER
