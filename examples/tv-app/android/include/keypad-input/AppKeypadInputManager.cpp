/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <json/json.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>

#include <string>

using namespace chip;
using namespace chip::app::Clusters::KeypadInput;
using Status = chip::Protocols::InteractionModel::Status;

namespace {

const std::string kSendKeyKeyCodeFieldId        = std::to_string(to_underlying(Commands::SendKey::Fields::kKeyCode));
const std::string kSendKeyResponseStatusFieldId = std::to_string(to_underlying(Commands::SendKeyResponse::Fields::kStatus));

// Same payload format that ContentAppCommandDelegate produces from the command TLV: {"<field id>":<value>}.
std::string SerializeSendKey(CECKeyCodeEnum keyCode)
{
    return "{\"" + kSendKeyKeyCodeFieldId + "\":" + std::to_string(to_underlying(keyCode)) + "}";
}

} // namespace

void AppKeypadInputManager::HandleSendKey(app::CommandResponseHelper<SendKeyResponseType> & helper, const CECKeyCodeEnum & keyCode)
{
    ChipLogProgress(Zcl, "AppKeypadInputManager::HandleSendKey endpoint %u keyCode %u", static_cast<unsigned>(mEndpointId),
                    static_cast<unsigned>(to_underlying(keyCode)));

    SendKeyResponseType response;
    response.status = StatusEnum::kInvalidKeyInCurrentState;

    if (mCommandDelegate == nullptr)
    {
        ChipLogError(Zcl, "AppKeypadInputManager: no ContentAppCommandDelegate");
        LogErrorOnFailure(helper.Success(response));
        return;
    }

    Json::Value jsonResponse;
    bool commandHandled = true;
    Status status;
    {
        // The content app answers through a blocking IPC round trip that can take seconds.
        DeviceLayer::StackUnlock unlock;
        status = mCommandDelegate->InvokeCommand(mEndpointId, chip::app::Clusters::KeypadInput::Id, Commands::SendKey::Id,
                                                 SerializeSendKey(keyCode), commandHandled, jsonResponse);
    }

    if (status != Status::Success)
    {
        ChipLogError(Zcl, "AppKeypadInputManager: content app returned status 0x%02x", to_underlying(status));
        LogErrorOnFailure(helper.Failure(status));
        return;
    }

    const Json::Value & statusValue = jsonResponse[kSendKeyResponseStatusFieldId];
    if (statusValue.isUInt())
    {
        response.status = static_cast<StatusEnum>(statusValue.asUInt());
    }
    else
    {
        ChipLogError(Zcl, "AppKeypadInputManager: content app response has no SendKeyResponse status");
    }
    LogErrorOnFailure(helper.Success(response));
}
