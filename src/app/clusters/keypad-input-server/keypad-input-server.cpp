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

#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>
#include <app-common/zap-generated/enums.h>
#include <app/CommandHandler.h>
#include <app/util/af.h>

using namespace chip;

EmberAfKeypadInputStatus keypadInputClusterSendKey(EmberAfKeypadInputCecKeyCode keyCode);

static void sendResponse(app::CommandHandler * command, EmberAfKeypadInputStatus keypadInputStatus)
{
    CHIP_ERROR err                   = CHIP_NO_ERROR;
    app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_KEYPAD_INPUT_CLUSTER_ID,
                                         ZCL_SEND_KEY_RESPONSE_COMMAND_ID, (app::CommandPathFlags::kEndpointIdValid) };
    TLV::TLVWriter * writer          = nullptr;

    VerifyOrExit(command != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(err = command->PrepareCommand(cmdParams));
    VerifyOrExit((writer = command->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(err = writer->Put(TLV::ContextTag(0), keypadInputStatus));
    SuccessOrExit(err = command->FinishCommand());

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode KeypadInputResponse command. Error:%s", ErrorStr(err));
    }
}

bool emberAfKeypadInputClusterSendKeyCallback(EndpointId endpoint, app::CommandHandler * command, uint8_t keyCode)
{
    EmberAfKeypadInputStatus status = keypadInputClusterSendKey(static_cast<EmberAfKeypadInputCecKeyCode>(keyCode));
    sendResponse(command, status);
    return true;
}
