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

#pragma once

#include "../../java/ContentAppCommandDelegate.h"
#include <app/clusters/account-login-server/account-login-server.h>

#include <app/util/af-types.h>

using chip::CharSpan;
using chip::EndpointId;
using chip::app::CommandResponseHelper;
using chip::Platform::CopyString;
using AccountLoginDelegate      = chip::app::Clusters::AccountLogin::Delegate;
using GetSetupPINResponse       = chip::app::Clusters::AccountLogin::Commands::GetSetupPINResponse::Type;
using ContentAppCommandDelegate = chip::AppPlatform::ContentAppCommandDelegate;

class AccountLoginManager : public AccountLoginDelegate
{
public:
    AccountLoginManager(ContentAppCommandDelegate * commandDelegate) : AccountLoginManager(commandDelegate, "tempPin123"){};
    AccountLoginManager(ContentAppCommandDelegate * commandDelegate, const char * setupPin);

    inline void SetSetupPin(char * setupPin) override { CopyString(mSetupPin, sizeof(mSetupPin), setupPin); };

    bool HandleLogin(const CharSpan & tempAccountIdentifierString, const CharSpan & setupPinString) override;
    bool HandleLogout() override;
    void HandleGetSetupPin(CommandResponseHelper<GetSetupPINResponse> & helper,
                           const CharSpan & tempAccountIdentifierString) override;
    void GetSetupPin(char * setupPin, size_t setupPinSize, const CharSpan & tempAccountIdentifierString) override;
    void SetEndpointId(EndpointId epId) { mEndpointId = epId; };

protected:
    static const size_t kSetupPinSize = 12;
    char mSetupPin[kSetupPinSize];

private:
    ContentAppCommandDelegate * mCommandDelegate;
    EndpointId mEndpointId;
};
