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
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventLogging.h>
#include <app/data-model/Encode.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <platform/CHIPDeviceConfig.h>
#include <protocols/interaction_model/StatusCode.h>

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/app-platform/ContentAppPlatform.h>
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AccountLogin;
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using namespace chip::AppPlatform;
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using chip::NodeId;
using chip::app::LogEvent;
using chip::app::Clusters::AccountLogin::Delegate;
using chip::Protocols::InteractionModel::Status;
using LoggedOutEvent = chip::app::Clusters::AccountLogin::Events::LoggedOut::Type;

static constexpr size_t kAccountLoginDeletageTableSize =
    MATTER_DM_ACCOUNT_LOGIN_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
static_assert(kAccountLoginDeletageTableSize <= kEmberInvalidEndpointIndex, "AccountLogin Delegate table size error");

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

    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, AccountLogin::Id, MATTER_DM_ACCOUNT_LOGIN_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kAccountLoginDeletageTableSize ? nullptr : gDelegateTable[ep]);
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
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, AccountLogin::Id, MATTER_DM_ACCOUNT_LOGIN_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found
    if (ep < kAccountLoginDeletageTableSize)
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
// Attribute Accessor Implementation

namespace {

class AccountLoginAttrAccess : public app::AttributeAccessInterface
{
public:
    AccountLoginAttrAccess() : app::AttributeAccessInterface(Optional<EndpointId>::Missing(), AccountLogin::Id) {}

    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadRevisionAttribute(EndpointId endpoint, app::AttributeValueEncoder & aEncoder, Delegate * delegate);
};

AccountLoginAttrAccess gAccountLoginAttrAccess;

CHIP_ERROR AccountLoginAttrAccess::Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder)
{
    EndpointId endpoint = aPath.mEndpointId;
    Delegate * delegate = GetDelegate(endpoint);

    switch (aPath.mAttributeId)
    {
    case app::Clusters::AccountLogin::Attributes::ClusterRevision::Id:
        return ReadRevisionAttribute(endpoint, aEncoder, delegate);
    default:
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AccountLoginAttrAccess::ReadRevisionAttribute(EndpointId endpoint, app::AttributeValueEncoder & aEncoder,
                                                         Delegate * delegate)
{
    uint16_t clusterRevision = delegate->GetClusterRevision(endpoint);
    return aEncoder.Encode(clusterRevision);
}

} // anonymous namespace

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

        command->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

bool emberAfAccountLoginClusterLoginCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                             const Commands::Login::DecodableType & commandData)
{
    CHIP_ERROR err               = CHIP_NO_ERROR;
    EndpointId endpoint          = commandPath.mEndpointId;
    Status status                = Status::Success;
    auto & tempAccountIdentifier = commandData.tempAccountIdentifier;
    auto & setupPin              = commandData.setupPIN;
    auto & nodeId                = commandData.node;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    if (!delegate->HandleLogin(tempAccountIdentifier, setupPin, nodeId))
    {
        status = Status::UnsupportedAccess;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfAccountLoginClusterLoginCallback error: %s", err.AsString());
        status = Status::Failure;
    }
    command->AddStatus(commandPath, status);
    return true;
}

bool emberAfAccountLoginClusterLogoutCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                              const Commands::Logout::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    Status status       = Status::Success;

    auto & nodeId = commandData.node;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    if (!delegate->HandleLogout(nodeId))
    {
        status = Status::Failure;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfAccountLoginClusterLogoutCallback error: %s", err.AsString());
        status = Status::Failure;
    }

    if (nodeId.HasValue())
    {
        // NodeId nodeId = getNodeId(commandObj);
        EventNumber eventNumber;
        LoggedOutEvent event{ .node = nodeId };
        CHIP_ERROR logEventError = LogEvent(event, endpoint, eventNumber);

        if (CHIP_NO_ERROR != logEventError)
        {
            ChipLogError(Zcl, "[Notify] Unable to send notify event: %s [endpointId=%d]", logEventError.AsString(), endpoint);
        }
    }

    commandObj->AddStatus(commandPath, status);
    return true;
}

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterAccountLoginPluginServerInitCallback()
{
    app::AttributeAccessInterfaceRegistry::Instance().Register(&gAccountLoginAttrAccess);
}
