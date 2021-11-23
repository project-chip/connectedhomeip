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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af.h>
#include <string>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AccountLogin;

bool accountLoginClusterIsUserLoggedIn(std::string requestTempAccountIdentifier, std::string requestSetupPin);
std::string accountLoginClusterGetSetupPin(std::string requestTempAccountIdentifier, EndpointId endpoint);

void sendResponse(app::CommandHandler * command, const char * responseSetupPin)
{
    CHIP_ERROR err                   = CHIP_NO_ERROR;
    app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, AccountLogin::Id,
                                         Commands::GetSetupPINResponse::Id, (app::CommandPathFlags::kEndpointIdValid) };
    TLV::TLVWriter * writer          = nullptr;
    SuccessOrExit(err = command->PrepareCommand(cmdParams));
    VerifyOrExit((writer = command->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(err = writer->PutString(TLV::ContextTag(0), responseSetupPin));
    SuccessOrExit(err = command->FinishCommand());
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode GetSetupPIN command. Error:%s", ErrorStr(err));
    }
}

bool emberAfAccountLoginClusterGetSetupPINCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                   const Commands::GetSetupPIN::DecodableType & commandData)
{
    auto & tempAccountIdentifier = commandData.tempAccountIdentifier;

    std::string tempAccountIdentifierString(tempAccountIdentifier.data(), tempAccountIdentifier.size());
    std::string responseSetupPin = accountLoginClusterGetSetupPin(tempAccountIdentifierString, emberAfCurrentEndpoint());
    sendResponse(command, responseSetupPin.c_str());
    return true;
}

bool emberAfAccountLoginClusterLoginCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                             const Commands::Login::DecodableType & commandData)
{
    auto & tempAccountIdentifier = commandData.tempAccountIdentifier;
    auto & tempSetupPin          = commandData.setupPIN;

    std::string tempAccountIdentifierString(tempAccountIdentifier.data(), tempAccountIdentifier.size());
    std::string tempSetupPinString(tempSetupPin.data(), tempSetupPin.size());
    bool isLoggedIn      = accountLoginClusterIsUserLoggedIn(tempAccountIdentifierString, tempSetupPinString);
    EmberAfStatus status = isLoggedIn ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_NOT_AUTHORIZED;
    if (!isLoggedIn)
    {
        ChipLogError(Zcl, "User is not authorized.");
    }
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

void MatterAccountLoginPluginServerInitCallback() {}
