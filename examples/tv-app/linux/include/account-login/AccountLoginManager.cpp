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

AccountLoginManager::AccountLoginManager(const char * setupPin)
{
    CopyString(mSetupPin, sizeof(mSetupPin), setupPin);
}

bool AccountLoginManager::HandleLogin(const CharSpan & tempAccountIdentifier, const CharSpan & setupPin)
{
    string tempAccountIdentifierString(tempAccountIdentifier.data(), tempAccountIdentifier.size());
    string setupPinString(setupPin.data(), setupPin.size());
    ChipLogProgress(Zcl, "temporary account id: %s", tempAccountIdentifierString.c_str());
    ChipLogProgress(Zcl, "setup pin %s", setupPinString.c_str());

    if (strcmp(mSetupPin, setupPinString.c_str()) == 0)
    {
        ChipLogProgress(Zcl, "AccountLoginManager::HandleLogin success");
        return true;
    }
    else
    {
        ChipLogProgress(Zcl, "AccountLoginManager::HandleLogin failed expected pin %s", mSetupPin);
        return false;
    }
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

    GetSetupPINResponse response;
    ChipLogProgress(Zcl, "temporary account id: %s returning pin: %s", tempAccountIdentifierString.c_str(), mSetupPin);

    response.setupPIN = CharSpan::fromCharString(mSetupPin);
    helper.Success(response);
}
