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
#include <app/util/af.h>
#include <platform/CHIPDeviceConfig.h>

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/app-platform/ContentAppPlatform.h>
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AccountLogin;
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using namespace chip::AppPlatform;
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using chip::app::Clusters::AccountLogin::Delegate;

static constexpr size_t kAccountLoginDeletageTableSize =
    EMBER_AF_ACCOUNT_LOGIN_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

// -----------------------------------------------------------------------------
// Delegate Implementation

namespace {

Delegate * gDelegateTable[kAccountLoginDeletageTableSize] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ContentApp * app = ContentAppPlatform::GetInstance().GetContentApp(endpoint);
    if (app != nullptr)
    {
        ChipLogProgress(Zcl, "AccountLogin returning ContentApp delegate for endpoint:%u", endpoint);
        return app->GetAccountLoginDelegate();
    }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ChipLogProgress(Zcl, "AccountLogin NOT returning ContentApp delegate for endpoint:%u", endpoint);

    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, AccountLogin::Id);
    return ((ep == 0xFFFF || ep >= EMBER_AF_ACCOUNT_LOGIN_CLUSTER_SERVER_ENDPOINT_COUNT) ? nullptr : gDelegateTable[ep]);
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        ChipLogProgress(Zcl, "Account Login has no delegate set for endpoint:%u", endpoint);
        return true;
    }
    return false;
}
} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace AccountLogin {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, AccountLogin::Id);
    // if endpoint is found and is not a dynamic endpoint
    if (ep != 0xFFFF && ep < EMBER_AF_ACCOUNT_LOGIN_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        gDelegateTable[ep] = delegate;
    }
    else
    {
    }
}

} // namespace AccountLogin
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Matter Framework Callbacks Implementation

bool emberAfAccountLoginClusterGetSetupPINCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                   const Commands::GetSetupPIN::DecodableType & commandData)
{
    CHIP_ERROR err               = CHIP_NO_ERROR;
    EndpointId endpoint          = commandPath.mEndpointId;
    auto & tempAccountIdentifier = commandData.tempAccountIdentifier;
    app::CommandResponseHelper<Commands::GetSetupPINResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        delegate->HandleGetSetupPin(responder, tempAccountIdentifier);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfAccountLoginClusterGetSetupPINCallback error: %s", err.AsString());

        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

bool emberAfAccountLoginClusterLoginCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                             const Commands::Login::DecodableType & commandData)
{
    CHIP_ERROR err               = CHIP_NO_ERROR;
    EndpointId endpoint          = commandPath.mEndpointId;
    EmberAfStatus status         = EMBER_ZCL_STATUS_SUCCESS;
    auto & tempAccountIdentifier = commandData.tempAccountIdentifier;
    auto & setupPin              = commandData.setupPIN;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    if (!delegate->HandleLogin(tempAccountIdentifier, setupPin))
    {
        status = EMBER_ZCL_STATUS_NOT_AUTHORIZED;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfAccountLoginClusterLoginCallback error: %s", err.AsString());
        status = EMBER_ZCL_STATUS_FAILURE;
    }
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfAccountLoginClusterLogoutCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                              const Commands::Logout::DecodableType & commandData)
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    EndpointId endpoint  = commandPath.mEndpointId;
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    if (!delegate->HandleLogout())
    {
        status = EMBER_ZCL_STATUS_FAILURE;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfAccountLoginClusterLogoutCallback error: %s", err.AsString());
        status = EMBER_ZCL_STATUS_FAILURE;
    }

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterAccountLoginPluginServerInitCallback() {}
