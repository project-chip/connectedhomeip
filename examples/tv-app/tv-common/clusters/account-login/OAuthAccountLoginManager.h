/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/account-login-server/account-login-server.h>

#include <lib/support/DLLUtil.h>
#include <lib/support/Span.h>
#include <system/SystemClock.h>
#include <system/SystemLayer.h>

using chip::CharSpan;
using chip::app::CommandResponseHelper;
using AccountLoginDelegate     = chip::app::Clusters::AccountLogin::Delegate;
using GetSetupPINResponse      = chip::app::Clusters::AccountLogin::Commands::GetSetupPINResponse::Type;
using GetDeviceAuthURIResponse = chip::app::Clusters::AccountLogin::Commands::GetDeviceAuthURIResponse::Type;

// AccountLogin delegate for a fake Content App that only supports the OAuth 2.0
// Device Authorization Grant flow. PIN-based login (GetSetupPIN/Login) is
// deliberately disabled: Login always fails and GetSetupPIN returns an inert
// placeholder. Requesting the device auth URI starts logged-out and flips
// OAuthLoggedIn to true on its own a few seconds later, simulating a user
// completing the out-of-band OAuth flow.
class DLL_EXPORT OAuthAccountLoginManager : public AccountLoginDelegate
{
public:
    inline void SetSetupPin(char * setupPin) override {};

    bool HandleLogin(const CharSpan & tempAccountIdentifierString, const CharSpan & setupPinString,
                     const chip::Optional<chip::NodeId> & nodeId) override;
    bool HandleLogout(const chip::Optional<chip::NodeId> & nodeId) override;
    void HandleGetSetupPin(CommandResponseHelper<GetSetupPINResponse> & helper,
                           const CharSpan & tempAccountIdentifierString) override;
    inline void GetSetupPin(char * setupPin, size_t setupPinSize, const CharSpan & tempAccountIdentifierString) override {};

    void HandleGetDeviceAuthURI(CommandResponseHelper<GetDeviceAuthURIResponse> & helper) override;
    bool GetOAuthLoggedIn(chip::EndpointId endpoint) override { return mOAuthLoggedIn; };

    uint16_t GetClusterRevision(chip::EndpointId endpoint) override;

    // Not part of the AccountLogin::Delegate contract. ContentAppImpl calls this
    // once the app-platform assigns this app's dynamic endpoint, so the delayed
    // login callback can report the attribute change on the right endpoint.
    void SetEndpointId(chip::EndpointId endpoint) { mEndpointId = endpoint; };

    ~OAuthAccountLoginManager() override;

private:
    static constexpr uint32_t kLoginDelaySeconds = 5;

    static void OnLoginTimerExpired(chip::System::Layer * systemLayer, void * context);

    bool mOAuthLoggedIn                        = false;
    chip::EndpointId mEndpointId               = chip::kInvalidEndpointId;
    static constexpr uint16_t kClusterRevision = 3;
};
