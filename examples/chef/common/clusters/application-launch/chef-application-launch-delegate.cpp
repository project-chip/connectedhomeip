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
#include <app/util/config.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ApplicationLauncher;

namespace {
static chip::app::Clusters::ApplicationLauncher::Chef::PlatformDelegate * gPlatformDelegate = nullptr;
static chip::IntrusiveList<chip::app::Clusters::ApplicationLauncher::Chef::AppDelegate> gAppDelegateList;
} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace ApplicationLauncher {
namespace Chef {

// PlatformDelegate Implementation

void PlatformDelegate::HandleLaunchApp(CommandResponseHelper<LauncherResponseType> & helper, const ByteSpan & data,
                                       const Application & application)
{
    ChipLogProgress(Zcl, "ApplicationLauncher::Chef::PlatformDelegate::HandleLaunchApp");
    LauncherResponseType response;
    response.status = StatusEnum::kSuccess;
    response.data   = chip::MakeOptional(data);
    helper.Success(response);
}

void PlatformDelegate::HandleStopApp(CommandResponseHelper<LauncherResponseType> & helper, const Application & application)
{
    ChipLogProgress(Zcl, "ApplicationLauncher::Chef::PlatformDelegate::HandleStopApp");
    LauncherResponseType response;
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}

void PlatformDelegate::HandleHideApp(CommandResponseHelper<LauncherResponseType> & helper, const Application & application)
{
    ChipLogProgress(Zcl, "ApplicationLauncher::Chef::PlatformDelegate::HandleHideApp");
    LauncherResponseType response;
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
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
    return aEncoder.EncodeNull();
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
        ApplicationBasic::CatalogVendorApp CatalogApp(*it->GetCatalogVendorApp());
        if (delegate->GetCatalogVendorApp()->Matches(CatalogApp))
        {
            ChipLogError(Zcl,
                         "ApplicationLauncher::Chef::PlatformDelegate::AddAppDelegate application (%d , %s) already registered",
                         CatalogApp.catalogVendorId, CatalogApp.applicationId);
            return CHIP_ERROR_ALREADY_INITIALIZED;
        }
    }
    mAppDelegateList.PushBack(delegate);
    return CHIP_NO_ERROR;
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
    LauncherResponseType response;
    response.status = StatusEnum::kSuccess;
    response.data   = chip::MakeOptional(data);
    helper.Success(response);
}

void AppDelegate::HandleStopApp(CommandResponseHelper<LauncherResponseType> & helper, const Application & application)
{
    ChipLogProgress(Zcl, "ApplicationLauncher::Chef::AppDelegate::HandleStopApp");
    LauncherResponseType response;
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}

void AppDelegate::HandleHideApp(CommandResponseHelper<LauncherResponseType> & helper, const Application & application)
{
    ChipLogProgress(Zcl, "ApplicationLauncher::Chef::AppDelegate::HandleHideApp");
    LauncherResponseType response;
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
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
    ChipLogProgress(Zcl, "ApplicationLauncher::Chef::AppDelegate::Match (%d , %s)", application.catalogVendorID,
                    application.applicationID.data());
    ApplicationBasic::CatalogVendorApp toCatalogApp(application.catalogVendorID, application.applicationID.data());
    return mAppBasicDelegate->GetCatalogVendorApp()->Matches(toCatalogApp);
}

} // namespace Chef
} // namespace ApplicationLauncher
} // namespace Clusters
} // namespace app
} // namespace chip
