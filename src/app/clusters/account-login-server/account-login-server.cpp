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

#include <app/clusters/account-login-server/account-login-delegate.h>
#include <app/clusters/account-login-server/account-login-server.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/ContentAppPlatform.h>
#include <app/util/af.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AccountLogin;

// -----------------------------------------------------------------------------
// Delegate Implementation

using chip::app::Clusters::AccountLogin::Delegate;
using namespace chip::AppPlatform;

namespace {

Delegate * gDelegate = NULL;

Delegate * GetDelegate(EndpointId endpoint)
{
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ContentApp * app = chip::AppPlatform::AppPlatform::GetInstance().GetContentAppByEndpointId(endpoint);
    if (app != NULL && app->GetAccountLoginDelegate() != NULL)
    {
        ChipLogError(Zcl, "AccountLogin returning ContentApp delegate for endpoint:%" PRIu16, endpoint);
        return app->GetAccountLoginDelegate();
    }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ChipLogError(Zcl, "AccountLogin NOT returning ContentApp delegate for endpoint:%" PRIu16, endpoint);

    return gDelegate;
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Account Login has no delegate set for endpoint:%" PRIu16, endpoint);
        return true;
    }
    return false;
}
} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace AccountLogin {

void SetDefaultDelegate(Delegate * delegate)
{
    gDelegate = delegate;
}

} // namespace AccountLogin
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Matter Framework Callbacks Implementation

bool emberAfAccountLoginClusterGetSetupPINRequestCallback(app::CommandHandler * command,
                                                          const app::ConcreteCommandPath & commandPath,
                                                          const Commands::GetSetupPINRequest::DecodableType & commandData)
{
    CHIP_ERROR err               = CHIP_NO_ERROR;
    EndpointId endpoint          = commandPath.mEndpointId;
    auto & tempAccountIdentifier = commandData.tempAccountIdentifier;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        Commands::GetSetupPINResponse::Type response = delegate->HandleGetSetupPin(tempAccountIdentifier);
        err                                          = command->AddResponseData(commandPath, response);
        SuccessOrExit(err);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfAccountLoginClusterGetSetupPINCallback error: %s", err.AsString());

        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

bool emberAfAccountLoginClusterLoginRequestCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                    const Commands::LoginRequest::DecodableType & commandData)
{
    CHIP_ERROR err               = CHIP_NO_ERROR;
    EndpointId endpoint          = commandPath.mEndpointId;
    auto & tempAccountIdentifier = commandData.tempAccountIdentifier;
    auto & setupPin              = commandData.setupPIN;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfAccountLoginClusterLoginRequestCallback error: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    bool isLoggedIn      = delegate->HandleLogin(tempAccountIdentifier, setupPin);
    EmberAfStatus status = isLoggedIn ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_NOT_AUTHORIZED;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfAccountLoginClusterLogoutRequestCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                     const Commands::LogoutRequest::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfAccountLoginClusterLogoutRequestCallback error: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    bool isLoggedOut     = delegate->HandleLogout();
    EmberAfStatus status = isLoggedOut ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_NOT_AUTHORIZED;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterAccountLoginPluginServerInitCallback() {}
