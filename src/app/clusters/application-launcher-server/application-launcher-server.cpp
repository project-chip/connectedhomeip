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

#include <app/clusters/application-basic-server/application-basic-delegate.h>
#include <app/clusters/application-basic-server/application-basic-server.h>
#include <app/clusters/application-launcher-server/application-launcher-delegate.h>
#include <app/clusters/application-launcher-server/application-launcher-server.h>

#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/app-platform/ContentAppPlatform.h>
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/data-model/Encode.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ApplicationLauncher;
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using namespace chip::AppPlatform;
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

// -----------------------------------------------------------------------------
// Delegate Implementation

using chip::app::Clusters::ApplicationBasic::CatalogVendorApp;
using chip::app::Clusters::ApplicationLauncher::Delegate;
using ApplicationStatusEnum = app::Clusters::ApplicationBasic::ApplicationStatusEnum;

namespace {

Delegate * gDelegateTable[EMBER_AF_APPLICATION_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ContentApp * app = ContentAppPlatform::GetInstance().GetContentApp(endpoint);
    if (app != nullptr)
    {
        ChipLogError(Zcl, "ApplicationLauncher returning ContentApp delegate for endpoint:%" PRIu16, endpoint);
        return app->GetApplicationLauncherDelegate();
    }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ChipLogError(Zcl, "ApplicationLauncher NOT returning ContentApp delegate for endpoint:%" PRIu16, endpoint);

    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ApplicationLauncher::Id);
    return ((ep == 0xFFFF || ep >= EMBER_AF_APPLICATION_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT) ? nullptr : gDelegateTable[ep]);
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
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ApplicationLauncher::Id);
    // if endpoint is found and is not a dynamic endpoint
    if (ep != 0xFFFF && ep < EMBER_AF_APPLICATION_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        gDelegateTable[ep] = delegate;
    }
    else
    {
    }
}

// this attribute should only be enabled for app platform instance (endpoint 1)
CHIP_ERROR Delegate::HandleGetCurrentApp(app::AttributeValueEncoder & aEncoder)
{
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    if (HasFeature(ApplicationLauncherFeature::kApplicationPlatform))
    {
        auto & platform = ContentAppPlatform::GetInstance();
        if (platform.HasCurrentApp())
        {
            ContentApp * app = platform.GetContentApp(platform.GetCurrentAppEndpointId());
            if (app != nullptr)
            {
                ApplicationEPType currentApp;
                CatalogVendorApp * vendorApp           = app->GetApplicationBasicDelegate()->GetCatalogVendorApp();
                std::string endpointStr                = std::to_string(app->GetEndpointId());
                currentApp.application.catalogVendorId = vendorApp->catalogVendorId;
                currentApp.application.applicationId   = CharSpan(vendorApp->applicationId, strlen(vendorApp->applicationId));
                currentApp.endpoint                    = CharSpan(endpointStr.c_str(), endpointStr.length());
                return aEncoder.Encode(currentApp);
            }
        }
    }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

    // TODO: change to return null once 13606 is fixed
    ApplicationEPType currentApp;
    currentApp.application.catalogVendorId = 123;
    currentApp.application.applicationId   = CharSpan("applicationId", strlen("applicationId"));
    currentApp.endpoint                    = CharSpan("endpointId", strlen("endpointId"));
    return aEncoder.Encode(currentApp);
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
    ApplicationLauncherAttrAccess() : app::AttributeAccessInterface(Optional<EndpointId>::Missing(), ApplicationLauncher::Id) {}

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
    return delegate->HandleGetCatalogList(aEncoder);
}

CHIP_ERROR ApplicationLauncherAttrAccess::ReadCurrentAppAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    return delegate->HandleGetCurrentApp(aEncoder);
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
    app::CommandResponseHelper<LauncherResponseType> responder(command, commandPath);

    std::string appId(application.applicationId.data(), application.applicationId.size());
    if (appId.length() == 0)
    {
        // chip-tool can't send structs from command line so treat data value as appid if appid is blank
        // TODO: fix this once chip-tool support sending structs from command line
        ChipLogError(Zcl, "ApplicationLauncher blank content id, taking data as appid");
        appId = std::string(data.data(), data.size());
    }
    CatalogVendorApp vendorApp(application.catalogVendorId, appId.c_str());

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
        // if the feature flag is APP_Plaform, then this is a call to endpoint 1 to launching an app
        //  1. Find target Content App and load if not loaded
        //  2. Set current app to Content App
        //  3. Set Content App status (basic cluster) to ACTIVE_VISIBLE_FOCUS
        //  4. Call launch app command on Content App
        if (delegate->HasFeature(ApplicationLauncherFeature::kApplicationPlatform))
        {
            ChipLogError(Zcl, "ApplicationLauncher has content platform feature");
            ContentApp * app = ContentAppPlatform::GetInstance().LoadContentApp(vendorApp);
            if (app == nullptr)
            {
                ChipLogError(Zcl, "ApplicationLauncher target app not found");
                LauncherResponseType response;
                response.data   = CharSpan("data", strlen("data"));
                response.status = StatusEnum::kAppNotAvailable;
                responder.Success(response);
                return true;
            }

            ContentAppPlatform::GetInstance().SetCurrentApp(app);

            ChipLogError(Zcl, "ApplicationLauncher handling launch on ContentApp");
            app->GetApplicationLauncherDelegate()->HandleLaunchApp(responder, data, application);
            return true;
        }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
       // otherwise, assume this is for managing status of the Content App on this endpoint
       //  1. Set Content App status (basic cluster) to ACTIVE_VISIBLE_FOCUS
       //  2. Call launch app command on the given endpoint

        ChipLogError(Zcl, "ApplicationLauncher no content platform feature");
        ApplicationBasic::Delegate * appBasic = ApplicationBasic::GetDefaultDelegate(endpoint);
        if (appBasic != nullptr)
        {
            ChipLogError(Zcl, "ApplicationLauncher setting basic cluster status to visible");
            appBasic->SetApplicationStatus(ApplicationStatusEnum::kActiveVisibleFocus);
        }

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
        ContentApp * app = ContentAppPlatform::GetInstance().LoadContentApp(vendorApp);
        if (app != nullptr)
        {
            ContentAppPlatform::GetInstance().SetCurrentApp(app);
        }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

        ChipLogError(Zcl, "ApplicationLauncher handling launch");
        delegate->HandleLaunchApp(responder, data, application);
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
    app::CommandResponseHelper<LauncherResponseType> responder(command, commandPath);

    std::string appId(application.applicationId.data(), application.applicationId.size());
    CatalogVendorApp vendorApp(application.catalogVendorId, appId.c_str());

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
        // if the feature flag is APP_Plaform, then this is a call to endpoint 1 to stop an app
        //  1. Find target Content App
        //  3. If this was the current app then stop it
        //  2. Set Content App status (basic cluster) to ACTIVE_STOPPED
        //  4. Call stop app command on Content App
        if (delegate->HasFeature(ApplicationLauncherFeature::kApplicationPlatform))
        {
            ChipLogError(Zcl, "ApplicationLauncher has content platform feature");
            ContentApp * app = ContentAppPlatform::GetInstance().LoadContentApp(vendorApp);
            if (app == nullptr)
            {
                ChipLogError(Zcl, "ApplicationLauncher target app not loaded");
                LauncherResponseType response;
                response.data   = CharSpan("data", strlen("data"));
                response.status = StatusEnum::kAppNotAvailable;
                responder.Success(response);
                return true;
            }

            ContentAppPlatform::GetInstance().UnsetIfCurrentApp(app);

            ChipLogError(Zcl, "ApplicationLauncher setting app status");
            app->GetApplicationBasicDelegate()->SetApplicationStatus(ApplicationStatusEnum::kStopped);

            ChipLogError(Zcl, "ApplicationLauncher handling stop on ContentApp");
            app->GetApplicationLauncherDelegate()->HandleStopApp(responder, application);
            return true;
        }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
       // otherwise, assume this is for managing status of the Content App on this endpoint
       //  1. Set Content App status (basic cluster) to ACTIVE_STOPPED
       //  2. Call launch app command on the given endpoint

        ChipLogError(Zcl, "ApplicationLauncher no content platform feature");

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
        ContentApp * app = ContentAppPlatform::GetInstance().GetContentApp(vendorApp);
        if (app != nullptr)
        {
            ContentAppPlatform::GetInstance().UnsetIfCurrentApp(app);
        }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

        ApplicationBasic::Delegate * appBasic = ApplicationBasic::GetDefaultDelegate(endpoint);
        if (appBasic != nullptr)
        {
            ChipLogError(Zcl, "ApplicationLauncher setting basic cluster status to stopped");
            appBasic->SetApplicationStatus(ApplicationStatusEnum::kStopped);
        }

        delegate->HandleStopApp(responder, application);
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
    app::CommandResponseHelper<LauncherResponseType> responder(command, commandPath);

    std::string appId(application.applicationId.data(), application.applicationId.size());
    CatalogVendorApp vendorApp(application.catalogVendorId, appId.c_str());

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
        // if the feature flag is APP_Plaform, then this is a call to endpoint 1 to stop an app
        //  1. Find target Content App
        //  3. If this was the current app then hide it
        //  2. Set Content App status (basic cluster) to ACTIVE_VISIBLE_NOT_FOCUS
        //  4. Call stop app command on Content App
        if (delegate->HasFeature(ApplicationLauncherFeature::kApplicationPlatform))
        {
            ChipLogError(Zcl, "ApplicationLauncher has content platform feature");
            ContentApp * app = ContentAppPlatform::GetInstance().GetContentApp(vendorApp);
            if (app == nullptr)
            {
                ChipLogError(Zcl, "ApplicationLauncher target app not loaded");
                LauncherResponseType response;
                response.data   = CharSpan("data", strlen("data"));
                response.status = StatusEnum::kAppNotAvailable;
                responder.Success(response);
                return true;
            }

            ContentAppPlatform::GetInstance().UnsetIfCurrentApp(app);

            ChipLogError(Zcl, "ApplicationLauncher handling stop on ContentApp");
            app->GetApplicationLauncherDelegate()->HandleHideApp(responder, application);
            return true;
        }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
       // otherwise, assume this is for managing status of the Content App on this endpoint
       //  1. Set Content App status (basic cluster) to ACTIVE_VISIBLE_NOT_FOCUS
       //  2. Call launch app command on the given endpoint

        ChipLogError(Zcl, "ApplicationLauncher no content platform feature");

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
        ContentApp * app = ContentAppPlatform::GetInstance().GetContentApp(vendorApp);
        if (app != nullptr)
        {
            ContentAppPlatform::GetInstance().UnsetIfCurrentApp(app);
        }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

        ApplicationBasic::Delegate * appBasic = ApplicationBasic::GetDefaultDelegate(endpoint);
        if (appBasic != nullptr)
        {
            ChipLogError(Zcl, "ApplicationLauncher setting basic cluster status to stopped");
            appBasic->SetApplicationStatus(ApplicationStatusEnum::kActiveHidden);
        }

        delegate->HandleHideApp(responder, application);
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
