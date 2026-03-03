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
#include <app/util/config.h>
#include <json/json.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace chip;
using namespace chip::app::Clusters::KeypadInput;
using Status = chip::Protocols::InteractionModel::Status;

namespace {

const auto sendKeyCodeFieldId = std::to_string(chip::to_underlying(Commands::SendKey::Fields::kKeyCode));

std::string serializeSendKeyCommand(Commands::SendKey::Type cmd)
{
    return R"({")" + sendKeyCodeFieldId + R"(":)" + std::to_string(chip::to_underlying(cmd.keyCode)) + R"(})";
}

} // namespace

AppKeypadInputManager::AppKeypadInputManager(ContentAppCommandDelegate * commandDelegate) : mCommandDelegate(commandDelegate) {}

void AppKeypadInputManager::HandleSendKey(CommandResponseHelper<SendKeyResponseType> & helper, const CECKeyCodeEnum & keyCode)
{
    ChipLogProgress(Zcl, "AppKeypadInputManager::HandleSendKey called for endpoint %d with keyCode %d", mEndpointId,
                    chip::to_underlying(keyCode));

    Commands::SendKeyResponse::Type response;

    if (mCommandDelegate == nullptr)
    {
        ChipLogError(Zcl, "CommandDelegate not found");
        response.status = chip::app::Clusters::KeypadInput::StatusEnum::kInvalidKeyInCurrentState;
        TEMPORARY_RETURN_IGNORED helper.Success(response);
        return;
    }

    Json::Value jsonResponse;
    bool commandHandled                   = true;
    Commands::SendKey::Type cmd = { keyCode };

    auto status = mCommandDelegate->InvokeCommand(mEndpointId, chip::app::Clusters::KeypadInput::Id, Commands::SendKey::Id,
                                                  serializeSendKeyCommand(cmd), commandHandled, jsonResponse);

    if (status == Status::Success)
    {
        // Parse the response to get the status
        std::string statusFieldId = std::to_string(chip::to_underlying(Commands::SendKeyResponse::Fields::kStatus));
        if (!jsonResponse[statusFieldId].empty() && jsonResponse[statusFieldId].isUInt())
        {
            response.status = static_cast<chip::app::Clusters::KeypadInput::StatusEnum>(jsonResponse[statusFieldId].asUInt());
        }
        else
        {
            ChipLogError(Zcl, "AppKeypadInputManager::HandleSendKey: Invalid response format");
            response.status = chip::app::Clusters::KeypadInput::StatusEnum::kInvalidKeyInCurrentState;
        }
    }
    else
    {
        ChipLogError(Zcl, "AppKeypadInputManager::HandleSendKey command failed with status: %d", chip::to_underlying(status));
        response.status = chip::app::Clusters::KeypadInput::StatusEnum::kInvalidKeyInCurrentState;
    }

    TEMPORARY_RETURN_IGNORED helper.Success(response);
}

uint32_t AppKeypadInputManager::GetFeatureMap(chip::EndpointId endpoint)
{
    if (endpoint >= MATTER_DM_CONTENT_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        return kEndpointFeatureMap;
    }

    uint32_t featureMap = 0;
    Attributes::FeatureMap::Get(endpoint, &featureMap);
    return featureMap;
}
