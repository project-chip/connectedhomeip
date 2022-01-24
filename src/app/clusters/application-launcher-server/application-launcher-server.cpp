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

#include <app/clusters/application-launcher-server/application-launcher-delegate.h>
#include <app/clusters/application-launcher-server/application-launcher-server.h>

#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model/Encode.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ApplicationLauncher;

// -----------------------------------------------------------------------------
// Delegate Implementation

using chip::app::Clusters::ApplicationLauncher::Delegate;

namespace {

Delegate * gDelegateTable[EMBER_AF_APPLICATION_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, chip::app::Clusters::ApplicationLauncher::Id);
    return (ep == 0xFFFF ? NULL : gDelegateTable[ep]);
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Application Launcher has no delegate set for endpoint:%" PRIu16, endpoint);
        return true;
    }
    return false;
}
} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace ApplicationLauncher {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, chip::app::Clusters::ApplicationLauncher::Id);
    if (ep != 0xFFFF)
    {
        gDelegateTable[ep] = delegate;
    }
    else
    {
    }
}

} // namespace ApplicationLauncher
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Attribute Accessor Implementation

namespace {

class ApplicationLauncherAttrAccess : public app::AttributeAccessInterface
{
public:
    ApplicationLauncherAttrAccess() :
        app::AttributeAccessInterface(Optional<EndpointId>::Missing(), chip::app::Clusters::ApplicationLauncher::Id)
    {}

    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadCatalogListAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadCurrentAppAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
};

ApplicationLauncherAttrAccess gApplicationLauncherAttrAccess;

CHIP_ERROR ApplicationLauncherAttrAccess::Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder)
{
    EndpointId endpoint = aPath.mEndpointId;
    Delegate * delegate = GetDelegate(endpoint);

    switch (aPath.mAttributeId)
    {
    case app::Clusters::ApplicationLauncher::Attributes::ApplicationLauncherList::Id: {
        if (isDelegateNull(delegate, endpoint))
        {
            return aEncoder.EncodeEmptyList();
        }

        return ReadCatalogListAttribute(aEncoder, delegate);
    }
    case app::Clusters::ApplicationLauncher::Attributes::ApplicationLauncherApp::Id: {
        if (isDelegateNull(delegate, endpoint))
        {
            return CHIP_NO_ERROR;
        }

        return ReadCurrentAppAttribute(aEncoder, delegate);
    }
    default: {
        break;
    }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ApplicationLauncherAttrAccess::ReadCatalogListAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    std::list<uint16_t> catalogList = delegate->HandleGetCatalogList();
    return aEncoder.EncodeList([catalogList](const auto & encoder) -> CHIP_ERROR {
        for (const auto & catalog : catalogList)
        {
            ReturnErrorOnFailure(encoder.Encode(catalog));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR ApplicationLauncherAttrAccess::ReadCurrentAppAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    Structs::ApplicationEP::Type currentApp = delegate->HandleGetCurrentApp();
    return aEncoder.Encode(currentApp);
}

} // anonymous namespace

// -----------------------------------------------------------------------------
// Matter Framework Callbacks Implementation

bool emberAfApplicationLauncherClusterLaunchAppRequestCallback(app::CommandHandler * command,
                                                               const app::ConcreteCommandPath & commandPath,
                                                               const Commands::LaunchAppRequest::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    auto & data         = commandData.data;
    auto & application  = commandData.application;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        Commands::LauncherResponse::Type response = delegate->HandleLaunchApp(data, application);
        err                                       = command->AddResponseData(commandPath, response);
        SuccessOrExit(err);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfApplicationLauncherClusterLaunchAppRequestCallback error: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

/**
 * @brief Application Launcher Cluster StopApp Command callback (from client)
 */
bool emberAfApplicationLauncherClusterStopAppRequestCallback(app::CommandHandler * command,
                                                             const app::ConcreteCommandPath & commandPath,
                                                             const Commands::StopAppRequest::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    auto & application  = commandData.application;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        Commands::LauncherResponse::Type response = delegate->HandleStopApp(application);
        err                                       = command->AddResponseData(commandPath, response);
        SuccessOrExit(err);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfApplicationLauncherClusterStopAppRequestCallback error: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

/**
 * @brief Application Launcher Cluster HideApp Command callback (from client)
 */
bool emberAfApplicationLauncherClusterHideAppRequestCallback(app::CommandHandler * command,
                                                             const app::ConcreteCommandPath & commandPath,
                                                             const Commands::HideAppRequest::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    auto & application  = commandData.application;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        Commands::LauncherResponse::Type response = delegate->HandleHideApp(application);
        err                                       = command->AddResponseData(commandPath, response);
        SuccessOrExit(err);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfApplicationLauncherClusterStopAppRequestCallback error: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterApplicationLauncherPluginServerInitCallback(void)
{
    registerAttributeAccessOverride(&gApplicationLauncherAttrAccess);
}
