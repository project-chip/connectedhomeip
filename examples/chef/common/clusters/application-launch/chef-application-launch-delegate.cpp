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

#include "chef-application-launch-delegate.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/application-basic-server/application-basic-delegate.h>
#include <app/clusters/application-launcher-server/application-launcher-server.h>
#include <app/reporting/reporting.h>
#include <app/util/config.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>

#if MATTER_DM_APPLICATION_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT > 0

using chip::EndpointId;
using chip::Span;
using chip::app::AttributeValueEncoder;
using chip::app::CommandResponseHelper;
using Application          = chip::app::Clusters::ApplicationLauncher::Structs::ApplicationStruct::Type;
using ApplicationEPType    = chip::app::Clusters::ApplicationLauncher::Structs::ApplicationEPStruct::Type;
using LauncherResponseType = chip::app::Clusters::ApplicationLauncher::Commands::LauncherResponse::Type;

namespace chip::app::Clusters::ApplicationLauncher::Chef {

// PlatformDelegate Implementation

void PlatformDelegate::HandleLaunchApp(CommandResponseHelper<LauncherResponseType> & helper, const ByteSpan & data,
                                       const Application & application)
{
    ChipLogProgress(Zcl, "ApplicationLauncher::Chef::PlatformDelegate::HandleLaunchApp (%d , %s)", application.catalogVendorID,
                    NullTerminated(application.applicationID).c_str());
    LauncherResponseType response;

    AppDelegate * app = FindAppDelegate(application);

    if (!app) // Did not find app in the list of managed apps
    {
        ChipLogError(Zcl, "ApplicationLauncher::Chef::PlatformDelegate::HandleLaunchApp : App not found");
        response.status = StatusEnum::kAppNotAvailable;
    }
    else if (app != mCurrentApp) // Launching a new app
    {
        ChipLogProgress(Zcl, "ApplicationLauncher::Chef::PlatformDelegate::HandleLaunchApp : Launching new app");
        if (mCurrentApp != nullptr)
        {
            mCurrentApp->SetApplicationStatus(ApplicationBasic::ApplicationStatusEnum::kActiveVisibleNotFocus);
            MatterReportingAttributeChangeCallback(mCurrentApp->GetEndpointId(), ApplicationBasic::Id,
                                                   ApplicationBasic::Attributes::Status::Id);
        }
        mCurrentApp = app;
        MatterReportingAttributeChangeCallback(mEndpointId, ApplicationLauncher::Id,
                                               ApplicationLauncher::Attributes::CurrentApp::Id);
        mCurrentApp->SetApplicationStatus(ApplicationBasic::ApplicationStatusEnum::kActiveVisibleFocus);
        MatterReportingAttributeChangeCallback(mCurrentApp->GetEndpointId(), ApplicationBasic::Id,
                                               ApplicationBasic::Attributes::Status::Id);
        response.status = StatusEnum::kSuccess;
    }
    else // Launching current in-focus app. Nothing to do.
    {
        response.status = StatusEnum::kSuccess;
    }

    response.data = chip::MakeOptional(data);
    LogErrorOnFailure(helper.Success(response));
}

void PlatformDelegate::HandleStopApp(CommandResponseHelper<LauncherResponseType> & helper, const Application & application)
{
    ChipLogProgress(Zcl, "ApplicationLauncher::Chef::PlatformDelegate::HandleStopApp (%d , %s)", application.catalogVendorID,
                    NullTerminated(application.applicationID).c_str());
    LauncherResponseType response;

    AppDelegate * app = FindAppDelegate(application);

    if (!app) // Did not find app in the list of managed apps
    {
        ChipLogError(Zcl, "ApplicationLauncher::Chef::PlatformDelegate::HandleStopApp : App not found");
        response.status = StatusEnum::kAppNotAvailable;
    }
    else
    {
        auto status = app->GetApplicationStatus();
        if (status == ApplicationBasic::ApplicationStatusEnum::kActiveVisibleFocus ||
            status == ApplicationBasic::ApplicationStatusEnum::kActiveHidden ||
            status == ApplicationBasic::ApplicationStatusEnum::kActiveVisibleNotFocus)
        {
            app->SetApplicationStatus(ApplicationBasic::ApplicationStatusEnum::kStopped);
            MatterReportingAttributeChangeCallback(app->GetEndpointId(), ApplicationBasic::Id,
                                                   ApplicationBasic::Attributes::Status::Id);
        }
        if (app == mCurrentApp)
        {
            mCurrentApp = nullptr;
            MatterReportingAttributeChangeCallback(mEndpointId, ApplicationLauncher::Id,
                                                   ApplicationLauncher::Attributes::CurrentApp::Id);
        }
        response.status = StatusEnum::kSuccess;
    }
    LogErrorOnFailure(helper.Success(response));
}

void PlatformDelegate::HandleHideApp(CommandResponseHelper<LauncherResponseType> & helper, const Application & application)
{
    ChipLogProgress(Zcl, "ApplicationLauncher::Chef::PlatformDelegate::HandleHideApp (%d , %s)", application.catalogVendorID,
                    NullTerminated(application.applicationID).c_str());
    LauncherResponseType response;

    AppDelegate * app = FindAppDelegate(application);
    if (!app) // Did not find app in the list of managed apps
    {
        ChipLogError(Zcl, "ApplicationLauncher::Chef::PlatformDelegate::HandleHideApp : App not found");
        response.status = StatusEnum::kAppNotAvailable;
    }
    else
    {
        auto status = app->GetApplicationStatus();
        if (status == ApplicationBasic::ApplicationStatusEnum::kActiveVisibleFocus ||
            status == ApplicationBasic::ApplicationStatusEnum::kActiveVisibleNotFocus)
        {
            app->SetApplicationStatus(ApplicationBasic::ApplicationStatusEnum::kActiveHidden);
            MatterReportingAttributeChangeCallback(app->GetEndpointId(), ApplicationBasic::Id,
                                                   ApplicationBasic::Attributes::Status::Id);
        }
        if (app == mCurrentApp)
        {
            mCurrentApp = nullptr;
            MatterReportingAttributeChangeCallback(mEndpointId, ApplicationLauncher::Id,
                                                   ApplicationLauncher::Attributes::CurrentApp::Id);
        }
        response.status = StatusEnum::kSuccess;
    }
    LogErrorOnFailure(helper.Success(response));
}

CHIP_ERROR PlatformDelegate::HandleGetCatalogList(app::AttributeValueEncoder & aEncoder)
{
    ChipLogProgress(Zcl, "ApplicationLauncher::Chef::PlatformDelegate::HandleGetCatalogList");
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (const auto & catalog : mCatalogList)
        {
            ReturnErrorOnFailure(encoder.Encode(catalog));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR PlatformDelegate::HandleGetCurrentApp(app::AttributeValueEncoder & aEncoder)
{
    ChipLogProgress(Zcl, "ApplicationLauncher::Chef::PlatformDelegate::HandleGetCurrentApp");
    if (!mCurrentApp)
    {
        ChipLogProgress(Zcl, "No app in focus.");
        return aEncoder.EncodeNull();
    }
    ChipLogProgress(Zcl, "Returning current app.");
    ApplicationEPType currentApp;
    ApplicationBasic::CatalogVendorApp * vendorApp = mCurrentApp->GetCatalogVendorApp();
    currentApp.application.catalogVendorID         = vendorApp->catalogVendorId;
    currentApp.application.applicationID           = CharSpan(vendorApp->applicationId, strlen(vendorApp->applicationId));
    currentApp.endpoint                            = Optional<EndpointId>(mCurrentApp->GetEndpointId());
    return aEncoder.Encode(currentApp);
}

CHIP_ERROR PlatformDelegate::AddAppDelegate(AppDelegate * delegate)
{
    ChipLogProgress(Zcl, "ApplicationLauncher::Chef::PlatformDelegate::AddAppDelegate");
    for (auto it = mAppDelegateList.begin(); it != mAppDelegateList.end(); ++it)
    {
        if (delegate->GetEndpointId() == it->GetEndpointId())
        {
            ChipLogError(Zcl, "ApplicationLauncher::Chef::PlatformDelegate::AddAppDelegate endpoint %d already registered",
                         delegate->GetEndpointId());
            return CHIP_ERROR_ALREADY_INITIALIZED;
        }
        ApplicationBasic::CatalogVendorApp CatalogApp(it->GetCatalogVendorApp());
        if (delegate->GetCatalogVendorApp()->Matches(CatalogApp))
        {
            ChipLogError(Zcl,
                         "ApplicationLauncher::Chef::PlatformDelegate::AddAppDelegate application (%d , %s) already registered",
                         CatalogApp.catalogVendorId, CatalogApp.applicationId);
            return CHIP_ERROR_ALREADY_INITIALIZED;
        }
    }
    ChipLogProgress(
        Zcl,
        "ApplicationLauncher::Chef::PlatformDelegate::AddAppDelegate : Adding new delegate Endpoint: %d, Application: (%d , %s)",
        delegate->GetEndpointId(), delegate->GetCatalogVendorApp()->catalogVendorId,
        delegate->GetCatalogVendorApp()->applicationId);
    mAppDelegateList.PushBack(delegate);
    return CHIP_NO_ERROR;
}

AppDelegate * PlatformDelegate::FindAppDelegate(const Application & application)
{
    ChipLogProgress(Zcl, "ApplicationLauncher::Chef::PlatformDelegate::FindAppDelegate (%d , %s)", application.catalogVendorID,
                    NullTerminated(application.applicationID).c_str());
    for (auto it = mAppDelegateList.begin(); it != mAppDelegateList.end(); ++it)
    {
        if (it->Match(application))
        {
            return it.operator->();
        }
    }
    return nullptr;
}

void PlatformDelegate::Register()
{
    ChipLogProgress(Zcl, "ApplicationLauncher::Chef::PlatformDelegate::Register");
    SetDefaultDelegate(mEndpointId, this);
}

// AppDelegate Implementation

void AppDelegate::HandleLaunchApp(CommandResponseHelper<LauncherResponseType> & helper, const ByteSpan & data,
                                  const Application & application)
{
    ChipLogProgress(Zcl, "ApplicationLauncher::Chef::AppDelegate::HandleLaunchApp");
    if (!mPlatformDelegate)
    {
        ChipLogError(Zcl, "ApplicationLauncher::Chef::AppDelegate::HandleLaunchApp : Platform delegate not initialised.");
        LogErrorOnFailure(helper.Failure(Protocols::InteractionModel::Status::InvalidInState));
        return;
    }
    Application targetApp;
    ApplicationBasic::CatalogVendorApp * vendorApp = GetCatalogVendorApp();
    targetApp.catalogVendorID                      = vendorApp->catalogVendorId;
    targetApp.applicationID                        = CharSpan(vendorApp->applicationId, strlen(vendorApp->applicationId));
    mPlatformDelegate->HandleLaunchApp(helper, data, targetApp);
}

void AppDelegate::HandleStopApp(CommandResponseHelper<LauncherResponseType> & helper, const Application & application)
{
    ChipLogProgress(Zcl, "ApplicationLauncher::Chef::AppDelegate::HandleStopApp");
    if (!mPlatformDelegate)
    {
        ChipLogError(Zcl, "ApplicationLauncher::Chef::AppDelegate::HandleStopApp : Platform delegate not initialised.");
        LogErrorOnFailure(helper.Failure(Protocols::InteractionModel::Status::InvalidInState));
        return;
    }
    Application targetApp;
    ApplicationBasic::CatalogVendorApp * vendorApp = GetCatalogVendorApp();
    targetApp.catalogVendorID                      = vendorApp->catalogVendorId;
    targetApp.applicationID                        = CharSpan(vendorApp->applicationId, strlen(vendorApp->applicationId));
    mPlatformDelegate->HandleStopApp(helper, targetApp);
}

void AppDelegate::HandleHideApp(CommandResponseHelper<LauncherResponseType> & helper, const Application & application)
{
    ChipLogProgress(Zcl, "ApplicationLauncher::Chef::AppDelegate::HandleHideApp");
    if (!mPlatformDelegate)
    {
        ChipLogError(Zcl, "ApplicationLauncher::Chef::AppDelegate::HandleHideApp : Platform delegate not initialised.");
        LogErrorOnFailure(helper.Failure(Protocols::InteractionModel::Status::InvalidInState));
        return;
    }
    Application targetApp;
    ApplicationBasic::CatalogVendorApp * vendorApp = GetCatalogVendorApp();
    targetApp.catalogVendorID                      = vendorApp->catalogVendorId;
    targetApp.applicationID                        = CharSpan(vendorApp->applicationId, strlen(vendorApp->applicationId));
    mPlatformDelegate->HandleHideApp(helper, targetApp);
}

CHIP_ERROR AppDelegate::HandleGetCatalogList(app::AttributeValueEncoder & aEncoder)
{
    ChipLogProgress(Zcl, "ApplicationLauncher::Chef::AppDelegate::HandleGetCatalogList returning unsupported");
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

void AppDelegate::Register()
{
    ChipLogProgress(Zcl, "ApplicationLauncher::Chef::AppDelegate::Register");
    SetDefaultDelegate(mEndpointId, this);
}

bool AppDelegate::Match(const Application & application)
{
    ChipLogProgress(Zcl, "ApplicationLauncher::Chef::AppDelegate::Match Checking (%d , %s)", application.catalogVendorID,
                    NullTerminated(application.applicationID).c_str());
    ApplicationBasic::CatalogVendorApp * vendorApp = GetCatalogVendorApp();
    if (vendorApp->catalogVendorId != application.catalogVendorID)
        return false;
    CharSpan appIdSpan = CharSpan(vendorApp->applicationId, strlen(vendorApp->applicationId));
    if (!appIdSpan.data_equal(application.applicationID))
    {
        return false;
    }
    ChipLogProgress(Zcl, "ApplicationLauncher::Chef::AppDelegate::Match returning true");
    return true;
}

} // namespace chip::app::Clusters::ApplicationLauncher::Chef

#endif // MATTER_DM_APPLICATION_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT
