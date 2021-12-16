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
/****************************************************************************
 * @file
 * @brief Routines for the Keypad Input plugin, the
 *server implementation of the Keypad Input cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/command-id.h>
#include <app-common/zap-generated/enums.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::KeypadInput;

EmberAfKeypadInputStatus keypadInputClusterSendKey(EmberAfKeypadInputCecKeyCode keyCode);

static void sendResponse(app::CommandHandler * command, EmberAfKeypadInputStatus keypadInputStatus)
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    app::ConcreteCommandPath path = { emberAfCurrentEndpoint(), KeypadInput::Id, KeypadInput::Commands::SendKeyResponse::Id };
    TLV::TLVWriter * writer       = nullptr;

    VerifyOrExit(command != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(err = command->PrepareCommand(path));
    VerifyOrExit((writer = command->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(err = writer->Put(TLV::ContextTag(0), keypadInputStatus));
    SuccessOrExit(err = command->FinishCommand());

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode KeypadInputResponse command. Error:%s", ErrorStr(err));
    }
}

bool emberAfKeypadInputClusterSendKeyCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                              const Commands::SendKey::DecodableType & commandData)
{
    auto & keyCode = commandData.keyCode;

    EmberAfKeypadInputStatus status = keypadInputClusterSendKey(static_cast<EmberAfKeypadInputCecKeyCode>(keyCode));
    sendResponse(command, status);
    return true;
}

void MatterKeypadInputPluginServerInitCallback() {}
