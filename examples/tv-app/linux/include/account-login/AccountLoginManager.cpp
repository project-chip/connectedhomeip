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
#include <app/util/af.h>

#include "gen/attribute-id.h"
#include "gen/attribute-type.h"
#include "gen/cluster-id.h"
#include "gen/command-id.h"
#include "gen/enums.h"

using namespace std;

bool AccountLoginManager::isUserLoggedIn(string requestTempAccountIdentifier, string requestSetupPin)
{
    for (auto it = accounts.cbegin(); it != accounts.cend(); ++it)
    {
        emberAfAccountLoginClusterPrintln("temporary account id: %s", it->first);
        emberAfAccountLoginClusterPrintln("setup pin %s", it->second);
    }

    if (accounts.find(requestTempAccountIdentifier) != accounts.end())
    {
        bool found = accounts[requestTempAccountIdentifier] == requestSetupPin;
        if (!found)
        {
            emberAfAccountLoginClusterPrintln("User is not logged in, failed to match request setup pin",
                                              EMBER_ZCL_STATUS_NOT_AUTHORIZED);
        }
        return found;
    }
    else
    {
        emberAfAccountLoginClusterPrintln("User is not logged in, failed to find temp account identifier",
                                          EMBER_ZCL_STATUS_NOT_AUTHORIZED);
        return false;
    }
}

void AccountLoginManager::setTempAccountIdentifierForPin(string tempAccountIdentifier, string setupPin)
{
    accounts[tempAccountIdentifier] = setupPin;
}

string AccountLoginManager::proxySetupPinRequest(string requestTempAccountIdentifier, chip::EndpointId endpoint)
{
    // TODO: Insert your code here to send temp account identifier request
    return "HtempPin123";
}

bool emberAfAccountLoginClusterGetSetupPINCallback(unsigned char * tempAccountIdentifier)
{
    string tempAccountIdentifierString(reinterpret_cast<char *>(tempAccountIdentifier));

    string responseSetupPin = AccountLoginManager().proxySetupPinRequest(tempAccountIdentifierString, emberAfCurrentEndpoint());
    emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT), ZCL_ACCOUNT_LOGIN_CLUSTER_ID,
                              ZCL_GET_SETUP_PIN_RESPONSE_COMMAND_ID, "s", &responseSetupPin);

    EmberStatus status = emberAfSendResponse();
    if (status != EMBER_SUCCESS)
    {
        emberAfAccountLoginClusterPrintln("Failed to send %s: 0x%X", "GetSetupPIN", status);
    }
    AccountLoginManager().GetInstance().setTempAccountIdentifierForPin(tempAccountIdentifierString, responseSetupPin);
    return true;
}

bool emberAfAccountLoginClusterLoginCallback(unsigned char * tempAccountIdentifier, unsigned char * tempSetupPin)
{
    string tempAccountIdentifierString(reinterpret_cast<char *>(tempAccountIdentifier));
    string tempSetupPinString(reinterpret_cast<char *>(tempSetupPin));
    bool isUserLoggedIn  = AccountLoginManager().GetInstance().isUserLoggedIn(tempAccountIdentifierString, tempSetupPinString);
    EmberAfStatus status = isUserLoggedIn ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_NOT_AUTHORIZED;
    emberAfAccountLoginClusterPrintln("User is not authorized.", EMBER_ZCL_STATUS_NOT_AUTHORIZED);
    emberAfSendImmediateDefaultResponse(status);
    return true;
}