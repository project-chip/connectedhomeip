/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

    ChipLogProgress(Zcl, "AccountLoginManager::HandleLogin failed expected pin %s", mSetupPin);
    return false;
}

bool AccountLoginManager::HandleLogout()
{
    // TODO: Insert your code here to send logout request
    ChipLogProgress(Zcl, "AccountLoginManager::HandleLogout success");
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
