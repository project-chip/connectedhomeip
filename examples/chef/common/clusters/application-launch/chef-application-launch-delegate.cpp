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

void AddApplicationLauncherPlatformDelegateForEndpoint(EndpointId endpoint, const Span<const uint16_t> catalogList)
{
    ChipLogProgress(Zcl, "ApplicationLauncher::Chef::AddApplicationLauncherPlatformDelegateForEndpoint endpoint=%d", endpoint);
    if (gPlatformDelegate != nullptr)
    {
        ChipLogError(Zcl, "ApplicationLauncher::Chef::PlatformDelegate already exists");
        return;
    }

    gPlatformDelegate = Platform::New<PlatformDelegate>(endpoint, catalogList);
    SetDefaultDelegate(endpoint, gPlatformDelegate);
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

void AddApplicationLauncherAppDelegateForEndpoint(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "ApplicationLauncher::Chef::AddApplicationLauncherAppDelegateForEndpoint endpoint=%d", endpoint);
    for (auto & delegate : gAppDelegateList)
    {
        if (delegate.GetEndpointId() == endpoint)
        {
            ChipLogError(Zcl, "ApplicationLauncher::Chef::AppDelegate already exists for endpoint %d", endpoint);
            return;
        }
    }

    AppDelegate * delegate = Platform::New<AppDelegate>(endpoint);
    gAppDelegateList.PushBack(delegate);
    SetDefaultDelegate(endpoint, delegate);
}

} // namespace Chef
} // namespace ApplicationLauncher
} // namespace Clusters
} // namespace app
} // namespace chip
