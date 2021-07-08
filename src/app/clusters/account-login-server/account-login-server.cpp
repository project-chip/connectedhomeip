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
 * @brief Routines for the Application Launcher plugin, the
 *server implementation of the Application Launcher cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app/CommandHandler.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/util/af.h>
#include <string>

bool accountLoginClusterIsUserLoggedIn(std::string requestTempAccountIdentifier, std::string requestSetupPin);
std::string accountLoginClusterGetSetupPin(std::string requestTempAccountIdentifier, chip::EndpointId endpoint);

void sendResponse(chip::app::CommandHandler * command, const char * responseSetupPin)
{
    CHIP_ERROR err                         = CHIP_NO_ERROR;
    chip::app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_ACCOUNT_LOGIN_CLUSTER_ID,
                                               ZCL_GET_SETUP_PIN_RESPONSE_COMMAND_ID,
                                               (chip::app::CommandPathFlags::kEndpointIdValid) };
    chip::TLV::TLVWriter * writer          = nullptr;
    SuccessOrExit(err = command->PrepareCommand(cmdParams));
    VerifyOrExit((writer = command->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(err = writer->PutString(chip::TLV::ContextTag(0), responseSetupPin));
    SuccessOrExit(err = command->FinishCommand());
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode GetSetupPIN command. Error:%s", chip::ErrorStr(err));
    }
}

bool emberAfAccountLoginClusterGetSetupPINCallback(chip::app::CommandHandler * command, uint8_t * tempAccountIdentifier)
{
    // TODO: char is not null terminated, verify this code once #7963 gets merged.
    std::string tempAccountIdentifierString(reinterpret_cast<char *>(tempAccountIdentifier));
    std::string responseSetupPin = accountLoginClusterGetSetupPin(tempAccountIdentifierString, emberAfCurrentEndpoint());
    sendResponse(command, responseSetupPin.c_str());
    return true;
}

bool emberAfAccountLoginClusterLoginCallback(chip::app::CommandHandler * command, uint8_t * tempAccountIdentifier,
                                             uint8_t * tempSetupPin)
{
    // TODO: char is not null terminated, verify this code once #7963 gets merged.
    std::string tempAccountIdentifierString(reinterpret_cast<char *>(tempAccountIdentifier));
    std::string tempSetupPinString(reinterpret_cast<char *>(tempSetupPin));
    bool isLoggedIn      = accountLoginClusterIsUserLoggedIn(tempAccountIdentifierString, tempSetupPinString);
    EmberAfStatus status = isLoggedIn ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_NOT_AUTHORIZED;
    if (!isLoggedIn)
    {
        ChipLogError(Zcl, "User is not authorized.");
    }
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
