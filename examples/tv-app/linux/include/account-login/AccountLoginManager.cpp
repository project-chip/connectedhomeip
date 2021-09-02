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
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>
#include <app-common/zap-generated/enums.h>
#include <app/Command.h>
#include <app/util/af.h>

using namespace std;

bool AccountLoginManager::isUserLoggedIn(string requestTempAccountIdentifier, string requestSetupPin)
{
    // TODO: Fix hardcoding length of strings
    requestTempAccountIdentifier = requestTempAccountIdentifier.substr(0, 4);
    requestSetupPin              = requestSetupPin.substr(0, 10);
    for (auto it = accounts.cbegin(); it != accounts.cend(); ++it)
    {
        ChipLogProgress(Zcl, "temporary account id: %s", it->first.c_str());
        ChipLogProgress(Zcl, "setup pin %s", it->second.c_str());
    }

    if (accounts.find(requestTempAccountIdentifier) != accounts.end())
    {
        bool found = accounts[requestTempAccountIdentifier] == requestSetupPin;
        if (!found)
        {
            ChipLogError(Zcl, "User is not logged in, failed to match request setup pin.");
        }
        return found;
    }
    else
    {
        ChipLogError(Zcl, "User is not logged in, failed to find temp account identifier.");
        return false;
    }
}

void AccountLoginManager::setTempAccountIdentifierForPin(string tempAccountIdentifier, string setupPin)
{
    // TODO: Fix hardcoding length of strings
    string tempId    = tempAccountIdentifier.substr(0, 4);
    accounts[tempId] = setupPin;
}

string AccountLoginManager::proxySetupPinRequest(string requestTempAccountIdentifier, chip::EndpointId endpoint)
{
    // TODO: Insert your code here to send temp account identifier request
    return "tempPin123";
}

bool accountLoginClusterIsUserLoggedIn(std::string requestTempAccountIdentifier, std::string requestSetupPin)
{
    return AccountLoginManager().GetInstance().isUserLoggedIn(requestTempAccountIdentifier, requestSetupPin);
}

std::string accountLoginClusterGetSetupPin(std::string requestTempAccountIdentifier, chip::EndpointId endpoint)
{
    string responseSetupPin = AccountLoginManager().proxySetupPinRequest(requestTempAccountIdentifier, endpoint);
    AccountLoginManager().GetInstance().setTempAccountIdentifierForPin(requestTempAccountIdentifier, responseSetupPin);
    return responseSetupPin;
}
