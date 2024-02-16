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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model/Encode.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <platform/CHIPDeviceConfig.h>

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/app-platform/ContentAppPlatform.h>
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ApplicationLauncher;
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using namespace chip::AppPlatform;
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using namespace chip::Uint8;

static constexpr size_t kApplicationLauncherDelegateTableSize =
    MATTER_DM_APPLICATION_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
static_assert(kApplicationLauncherDelegateTableSize <= kEmberInvalidEndpointIndex, "ApplicationLauncher Delegate table size error");

// -----------------------------------------------------------------------------
// Delegate Implementation

using chip::app::Clusters::ApplicationBasic::CatalogVendorApp;
using chip::app::Clusters::ApplicationLauncher::Delegate;
using ApplicationStatusEnum = app::Clusters::ApplicationBasic::ApplicationStatusEnum;
using chip::Protocols::InteractionModel::Status;

namespace {

Delegate * gDelegateTable[kApplicationLauncherDelegateTableSize] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ContentApp * app = ContentAppPlatform::GetInstance().GetContentApp(endpoint);
    if (app != nullptr)
    {
        ChipLogProgress(Zcl, "ApplicationLauncher returning ContentApp delegate for endpoint:%u", endpoint);
        return app->GetApplicationLauncherDelegate();
    }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ChipLogProgress(Zcl, "ApplicationLauncher NOT returning ContentApp delegate for endpoint:%u", endpoint);

    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, ApplicationLauncher::Id,
                                                       MATTER_DM_APPLICATION_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kApplicationLauncherDelegateTableSize ? nullptr : gDelegateTable[ep]);
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        ChipLogProgress(Zcl, "Application Launcher has no delegate set for endpoint:%u", endpoint);
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
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, ApplicationLauncher::Id,
                                                       MATTER_DM_APPLICATION_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found
    if (ep < kApplicationLauncherDelegateTableSize)
    {
        gDelegateTable[ep] = delegate;
    }
    else
    {
    }
}

bool HasFeature(chip::EndpointId endpoint, Feature feature)
{
    bool hasFeature     = false;
    uint32_t featureMap = 0;

    Status status = Attributes::FeatureMap::Get(endpoint, &featureMap);
    if (Status::Success == status)
    {
        hasFeature = (featureMap & chip::to_underlying(feature));
    }

    return hasFeature;
}

// this attribute should only be enabled for app platform instance (endpoint 1)
CHIP_ERROR Delegate::HandleGetCurrentApp(app::AttributeValueEncoder & aEncoder)
{
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    if (HasFeature(Feature::kApplicationPlatform))
    {
        auto & platform = ContentAppPlatform::GetInstance();
        if (platform.HasCurrentApp())
        {
            ContentApp * app = platform.GetContentApp(platform.GetCurrentAppEndpointId());
            if (app != nullptr)
            {
                ApplicationEPType currentApp;
                CatalogVendorApp * vendorApp           = app->GetApplicationBasicDelegate()->GetCatalogVendorApp();
                currentApp.application.catalogVendorID = vendorApp->catalogVendorId;
                currentApp.application.applicationID   = CharSpan(vendorApp->applicationId, strlen(vendorApp->applicationId));
                currentApp.endpoint                    = Optional<EndpointId>(app->GetEndpointId());
                return aEncoder.Encode(currentApp);
            }
        }
    }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

    return aEncoder.EncodeNull();
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
    case app::Clusters::ApplicationLauncher::Attributes::CatalogList::Id: {
        if (isDelegateNull(delegate, endpoint))
        {
            return aEncoder.EncodeEmptyList();
        }

        return ReadCatalogListAttribute(aEncoder, delegate);
    }
    case app::Clusters::ApplicationLauncher::Attributes::CurrentApp::Id: {
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

bool emberAfApplicationLauncherClusterLaunchAppCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                        const Commands::LaunchApp::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    auto & data         = commandData.data;
    auto & application  = commandData.application;
    app::CommandResponseHelper<LauncherResponseType> responder(command, commandPath);

    // TODO: We should be able to not have an `application` if the AP feature is
    // off: https://github.com/project-chip/connectedhomeip/issues/24595
    if (!application.HasValue())
    {
        command->AddStatus(commandPath, Protocols::InteractionModel::Status::InvalidCommand);
        return true;
    }
    std::string appId(application.Value().applicationID.data(), application.Value().applicationID.size());
    CatalogVendorApp vendorApp(application.Value().catalogVendorID, appId.c_str());

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
        // if the feature flag is APP_Plaform, then this is a call to endpoint 1 to launching an app
        //  1. Find target Content App and load if not loaded
        //  2. Set current app to Content App
        //  3. Set Content App status (basic cluster) to ACTIVE_VISIBLE_FOCUS
        //  4. Call launch app command on Content App
        if (delegate->HasFeature(Feature::kApplicationPlatform))
        {
            ChipLogError(Zcl, "ApplicationLauncher has content platform feature");
            ContentApp * app = ContentAppPlatform::GetInstance().LoadContentApp(&vendorApp);
            if (app == nullptr)
            {
                ChipLogError(Zcl, "ApplicationLauncher target app not found");
                LauncherResponseType response;
                response.status = StatusEnum::kAppNotAvailable;
                responder.Success(response);
                return true;
            }

            ContentAppPlatform::GetInstance().SetCurrentApp(app);

            ChipLogError(Zcl, "ApplicationLauncher handling launch on ContentApp");
            app->GetApplicationLauncherDelegate()->HandleLaunchApp(responder, data.HasValue() ? data.Value() : ByteSpan(),
                                                                   application.Value());
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
        ContentApp * app = ContentAppPlatform::GetInstance().LoadContentApp(&vendorApp);
        if (app != nullptr)
        {
            ContentAppPlatform::GetInstance().SetCurrentApp(app);
        }
        else
        {
            ChipLogError(Zcl, "ApplicationLauncher target app not found");
            LauncherResponseType response;
            response.status = StatusEnum::kAppNotAvailable;
            responder.Success(response);
            return true;
        }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

        ChipLogError(Zcl, "ApplicationLauncher handling launch");
        delegate->HandleLaunchApp(responder, data.HasValue() ? data.Value() : ByteSpan(), application.Value());
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfApplicationLauncherClusterLaunchAppCallback error: %s", err.AsString());
        command->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

/**
 * @brief Application Launcher Cluster StopApp Command callback (from client)
 */
bool emberAfApplicationLauncherClusterStopAppCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                      const Commands::StopApp::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    auto & application  = commandData.application;
    app::CommandResponseHelper<LauncherResponseType> responder(command, commandPath);

    // TODO: We should be able to not have an `application` if the AP feature is
    // off: https://github.com/project-chip/connectedhomeip/issues/24595
    if (!application.HasValue())
    {
        command->AddStatus(commandPath, Protocols::InteractionModel::Status::InvalidCommand);
        return true;
    }
    std::string appId(application.Value().applicationID.data(), application.Value().applicationID.size());
    CatalogVendorApp vendorApp(application.Value().catalogVendorID, appId.c_str());

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
        // if the feature flag is APP_Plaform, then this is a call to endpoint 1 to stop an app
        //  1. Find target Content App
        //  3. If this was the current app then stop it
        //  2. Set Content App status (basic cluster) to ACTIVE_STOPPED
        //  4. Call stop app command on Content App
        if (delegate->HasFeature(Feature::kApplicationPlatform))
        {
            ChipLogError(Zcl, "ApplicationLauncher has content platform feature");
            ContentApp * app = ContentAppPlatform::GetInstance().LoadContentApp(&vendorApp);
            if (app == nullptr)
            {
                ChipLogError(Zcl, "ApplicationLauncher target app not loaded");
                LauncherResponseType response;
                response.status = StatusEnum::kAppNotAvailable;
                responder.Success(response);
                return true;
            }

            ContentAppPlatform::GetInstance().UnsetIfCurrentApp(app);

            ChipLogError(Zcl, "ApplicationLauncher setting app status");
            app->GetApplicationBasicDelegate()->SetApplicationStatus(ApplicationStatusEnum::kStopped);

            ChipLogError(Zcl, "ApplicationLauncher handling stop on ContentApp");
            app->GetApplicationLauncherDelegate()->HandleStopApp(responder, application.Value());
            return true;
        }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
       // otherwise, assume this is for managing status of the Content App on this endpoint
       //  1. Set Content App status (basic cluster) to ACTIVE_STOPPED
       //  2. Call launch app command on the given endpoint

        ChipLogError(Zcl, "ApplicationLauncher no content platform feature");

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
        ContentApp * app = ContentAppPlatform::GetInstance().GetContentApp(&vendorApp);
        if (app != nullptr)
        {
            ContentAppPlatform::GetInstance().UnsetIfCurrentApp(app);
            app->GetApplicationBasicDelegate()->SetApplicationStatus(ApplicationStatusEnum::kStopped);
        }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

        ApplicationBasic::Delegate * appBasic = ApplicationBasic::GetDefaultDelegate(endpoint);
        if (appBasic != nullptr)
        {
            ChipLogError(Zcl, "ApplicationLauncher setting basic cluster status to stopped");
            appBasic->SetApplicationStatus(ApplicationStatusEnum::kStopped);
        }

        delegate->HandleStopApp(responder, application.Value());
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfApplicationLauncherClusterStopAppCallback error: %s", err.AsString());
        command->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

/**
 * @brief Application Launcher Cluster HideApp Command callback (from client)
 */
bool emberAfApplicationLauncherClusterHideAppCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                      const Commands::HideApp::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    auto & application  = commandData.application;
    app::CommandResponseHelper<LauncherResponseType> responder(command, commandPath);

    // TODO: We should be able to not have an `application` if the AP feature is
    // off: https://github.com/project-chip/connectedhomeip/issues/24595
    if (!application.HasValue())
    {
        command->AddStatus(commandPath, Protocols::InteractionModel::Status::InvalidCommand);
        return true;
    }
    std::string appId(application.Value().applicationID.data(), application.Value().applicationID.size());
    CatalogVendorApp vendorApp(application.Value().catalogVendorID, appId.c_str());

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
        // if the feature flag is APP_Plaform, then this is a call to endpoint 1 to stop an app
        //  1. Find target Content App
        //  3. If this was the current app then hide it
        //  2. Set Content App status (basic cluster) to ACTIVE_VISIBLE_NOT_FOCUS
        //  4. Call stop app command on Content App
        if (delegate->HasFeature(Feature::kApplicationPlatform))
        {
            ChipLogError(Zcl, "ApplicationLauncher has content platform feature");
            ContentApp * app = ContentAppPlatform::GetInstance().GetContentApp(&vendorApp);
            if (app == nullptr)
            {
                ChipLogError(Zcl, "ApplicationLauncher target app not loaded");
                LauncherResponseType response;
                response.status = StatusEnum::kAppNotAvailable;
                responder.Success(response);
                return true;
            }

            ContentAppPlatform::GetInstance().UnsetIfCurrentApp(app);

            ChipLogError(Zcl, "ApplicationLauncher handling stop on ContentApp");
            app->GetApplicationLauncherDelegate()->HandleHideApp(responder, application.Value());
            return true;
        }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
       // otherwise, assume this is for managing status of the Content App on this endpoint
       //  1. Set Content App status (basic cluster) to ACTIVE_VISIBLE_NOT_FOCUS
       //  2. Call launch app command on the given endpoint

        ChipLogError(Zcl, "ApplicationLauncher no content platform feature");

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
        ContentApp * app = ContentAppPlatform::GetInstance().GetContentApp(&vendorApp);
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

        delegate->HandleHideApp(responder, application.Value());
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfApplicationLauncherClusterStopAppCallback error: %s", err.AsString());
        command->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterApplicationLauncherPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gApplicationLauncherAttrAccess);
}
