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

#include "OAuthAccountLoginManager.h"
#include <app/CommandHandler.h>
#include <app/reporting/reporting.h>
#include <clusters/AccountLogin/Metadata.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app::Clusters::AccountLogin;

bool OAuthAccountLoginManager::HandleLogin(const CharSpan & tempAccountIdentifier, const CharSpan & setupPin,
                                           const chip::Optional<chip::NodeId> & nodeId)
{
    // PIN-based login is not supported by this app - only the OAuth device
    // authorization grant flow (see HandleGetDeviceAuthURI) can log a user in.
    ChipLogProgress(Zcl, "OAuthAccountLoginManager::HandleLogin rejected: PIN login is disabled for this app");
    return false;
}

bool OAuthAccountLoginManager::HandleLogout(const chip::Optional<chip::NodeId> & nodeId)
{
    DeviceLayer::SystemLayer().CancelTimer(SimulateAsyncLoginSuccess, this);
    if (mOAuthLoggedIn)
    {
        mOAuthLoggedIn = false;
        if (mEndpointId != kInvalidEndpointId)
        {
            MatterReportingAttributeChangeCallback(mEndpointId, Id, Attributes::OAuthLoggedIn::Id);
        }
    }
    ChipLogProgress(Zcl, "OAuthAccountLoginManager::HandleLogout success");
    return true;
}

void OAuthAccountLoginManager::HandleGetSetupPin(CommandResponseHelper<GetSetupPINResponse> & helper,
                                                 const CharSpan & tempAccountIdentifier)
{
    // PIN-based login is disabled for this app; respond with an inert placeholder
    // that can never be used to successfully call Login.
    GetSetupPINResponse response;
    response.setupPIN = "N/A"_span;
    LogErrorOnFailure(helper.Success(response));
}

void OAuthAccountLoginManager::HandleGetDeviceAuthURI(CommandResponseHelper<GetDeviceAuthURIResponse> & helper)
{
    ChipLogProgress(Zcl, "OAuthAccountLoginManager::HandleGetDeviceAuthURI: will report logged-in in %u seconds",
                    static_cast<unsigned>(kLoginDelaySeconds));

    DeviceLayer::SystemLayer().CancelTimer(SimulateAsyncLoginSuccess, this);
    CHIP_ERROR err =
        DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(kLoginDelaySeconds), SimulateAsyncLoginSuccess, this);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "OAuthAccountLoginManager::HandleGetDeviceAuthURI failed to start login timer: %" CHIP_ERROR_FORMAT,
                    err.Format());
        LogErrorOnFailure(helper.Failure(Protocols::InteractionModel::Status::Failure));
        return;
    }

    GetDeviceAuthURIResponse response;
    response.userCode        = "ABCD-EFGH"_span;
    response.verificationURI = "https://example.com/device"_span;
    response.expiresIn       = 1800;
    response.interval        = 5;
    LogErrorOnFailure(helper.Success(response));
}

void OAuthAccountLoginManager::SimulateAsyncLoginSuccess(System::Layer * systemLayer, void * context)
{
    auto * self          = reinterpret_cast<OAuthAccountLoginManager *>(context);
    self->mOAuthLoggedIn = true;
    ChipLogProgress(Zcl, "OAuthAccountLoginManager: OAuth login completed asynchronously");

    if (self->mEndpointId != kInvalidEndpointId)
    {
        MatterReportingAttributeChangeCallback(self->mEndpointId, Id, Attributes::OAuthLoggedIn::Id);
    }
}

uint16_t OAuthAccountLoginManager::GetClusterRevision(chip::EndpointId endpoint)
{
    return chip::app::Clusters::AccountLogin::kRevision;
}

OAuthAccountLoginManager::~OAuthAccountLoginManager()
{
    DeviceLayer::SystemLayer().CancelTimer(SimulateAsyncLoginSuccess, this);
}
