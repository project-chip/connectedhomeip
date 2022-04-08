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

#include "AccountLoginManager.h"
#include <app/CommandHandler.h>
#include <app/util/af.h>

using namespace std;
using namespace chip::app::Clusters::AccountLogin;

bool AccountLoginManager::HandleLogin(const chip::CharSpan & tempAccountIdentifier, const chip::CharSpan & setupPin)
{
    string tempAccountIdentifierString(tempAccountIdentifier.data(), tempAccountIdentifier.size());
    string setupPinString(setupPin.data(), setupPin.size());
    ChipLogProgress(Zcl, "temporary account id: %s", tempAccountIdentifierString.c_str());
    ChipLogProgress(Zcl, "setup pin %s", setupPinString.c_str());

    // TODO: Insert your code here to handle login request
    return true;
}

bool AccountLoginManager::HandleLogout()
{
    // TODO: Insert your code here to send logout request
    return true;
}

void AccountLoginManager::HandleGetSetupPin(CommandResponseHelper<GetSetupPINResponse> & helper,
                                            const CharSpan & tempAccountIdentifier)
{
    string tempAccountIdentifierString(tempAccountIdentifier.data(), tempAccountIdentifier.size());
    ChipLogProgress(Zcl, "temporary account id: %s", tempAccountIdentifierString.c_str());
    // TODO: Insert your code here to handle get setup pin
    Commands::GetSetupPINResponse::Type response;
    response.setupPIN = chip::CharSpan::fromCharString("tempPin123");
    helper.Success(response);
}
