/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "KeypadInputManager.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/config.h>

using namespace chip;
using namespace chip::app::Clusters::KeypadInput;

void KeypadInputManager::HandleSendKey(CommandResponseHelper<SendKeyResponseType> & helper, const CecKeyCodeType & keycCode)
{
    // TODO: Insert code here
    Commands::SendKeyResponse::Type response;

    switch (keycCode)
    {
    case CecKeyCodeType::kUp:
        response.status = chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kSuccess;
        break;
    case CecKeyCodeType::kDown:
        response.status = chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kSuccess;
        break;
    case CecKeyCodeType::kLeft:
        response.status = chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kSuccess;
        break;
    case CecKeyCodeType::kRight:
        response.status = chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kSuccess;
        break;
    case CecKeyCodeType::kSelect:
        response.status = chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kSuccess;
        break;
    case CecKeyCodeType::kBackward:
        response.status = chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kSuccess;
        break;
    case CecKeyCodeType::kExit:
        response.status = chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kSuccess;
        break;
    case CecKeyCodeType::kRootMenu:
        response.status = chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kSuccess;
        break;
    case CecKeyCodeType::kSetupMenu:
        response.status = chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kSuccess;
        break;
    case CecKeyCodeType::kEnter:
        response.status = chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kSuccess;
        break;
    case CecKeyCodeType::kNumber0OrNumber10:
        response.status = chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kSuccess;
        break;
    case CecKeyCodeType::kNumbers1:
        response.status = chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kSuccess;
        break;
    case CecKeyCodeType::kNumbers2:
        response.status = chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kSuccess;
        break;
    case CecKeyCodeType::kNumbers3:
        response.status = chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kSuccess;
        break;
    case CecKeyCodeType::kNumbers4:
        response.status = chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kSuccess;
        break;
    case CecKeyCodeType::kNumbers5:
        response.status = chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kSuccess;
        break;
    case CecKeyCodeType::kNumbers6:
        response.status = chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kSuccess;
        break;
    case CecKeyCodeType::kNumbers7:
        response.status = chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kSuccess;
        break;
    case CecKeyCodeType::kNumbers8:
        response.status = chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kSuccess;
        break;
    case CecKeyCodeType::kNumbers9:
        response.status = chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kSuccess;
        break;
    default:
        response.status = chip::app::Clusters::KeypadInput::KeypadInputStatusEnum::kUnsupportedKey;
    }

    helper.Success(response);
}

uint32_t KeypadInputManager::GetFeatureMap(chip::EndpointId endpoint)
{
    if (endpoint >= EMBER_AF_CONTENT_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        return mDynamicEndpointFeatureMap;
    }

    uint32_t featureMap = 0;
    Attributes::FeatureMap::Get(endpoint, &featureMap);
    return featureMap;
}
