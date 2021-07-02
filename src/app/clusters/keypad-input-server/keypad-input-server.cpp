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

#include <app/Command.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/common/gen/enums.h>
#include <app/util/af.h>

EmberAfKeypadInputStatus keypadInputClusterSendKey(EmberAfKeypadInputCecKeyCode keyCode);

static void sendResponse(chip::app::Command * command, EmberAfKeypadInputStatus keypadInputStatus)
{
    CHIP_ERROR err                         = CHIP_NO_ERROR;
    chip::app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_KEYPAD_INPUT_CLUSTER_ID,
                                               ZCL_SEND_KEY_RESPONSE_COMMAND_ID, (chip::app::CommandPathFlags::kEndpointIdValid) };
    chip::TLV::TLVWriter * writer          = nullptr;

    VerifyOrExit(command != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(err = command->PrepareCommand(cmdParams));
    VerifyOrExit((writer = command->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(err = writer->Put(chip::TLV::ContextTag(0), keypadInputStatus));
    SuccessOrExit(err = command->FinishCommand());

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode KeypadInputResponse command. Error:%s", chip::ErrorStr(err));
    }
}

bool emberAfKeypadInputClusterSendKeyCallback(chip::app::Command * command, uint8_t keyCode)
{
    EmberAfKeypadInputStatus status = keypadInputClusterSendKey(static_cast<EmberAfKeypadInputCecKeyCode>(keyCode));
    sendResponse(command, status);
    return true;
}
