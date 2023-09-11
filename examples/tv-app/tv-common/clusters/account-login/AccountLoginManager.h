/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/clusters/account-login-server/account-login-server.h>

#include <app/util/af-types.h>

using chip::CharSpan;
using chip::app::CommandResponseHelper;
using chip::Platform::CopyString;
using AccountLoginDelegate = chip::app::Clusters::AccountLogin::Delegate;
using GetSetupPINResponse  = chip::app::Clusters::AccountLogin::Commands::GetSetupPINResponse::Type;

class AccountLoginManager : public AccountLoginDelegate
{
public:
    AccountLoginManager() : AccountLoginManager("tempPin123"){};
    AccountLoginManager(const char * setupPin);

    inline void SetSetupPin(char * setupPin) override { CopyString(mSetupPin, sizeof(mSetupPin), setupPin); };

    bool HandleLogin(const CharSpan & tempAccountIdentifierString, const CharSpan & setupPinString) override;
    bool HandleLogout() override;
    void HandleGetSetupPin(CommandResponseHelper<GetSetupPINResponse> & helper,
                           const CharSpan & tempAccountIdentifierString) override;
    inline void GetSetupPin(char * setupPin, size_t setupPinSize, const CharSpan & tempAccountIdentifierString) override
    {
        CopyString(setupPin, setupPinSize, mSetupPin);
    };

protected:
    static const size_t kSetupPinSize = 12;
    char mSetupPin[kSetupPinSize];
};
