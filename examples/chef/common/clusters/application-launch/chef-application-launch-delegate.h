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

#pragma once

#include <app/clusters/application-launcher-server/application-launcher-delegate.h>
#include <app/util/config.h>
#include <lib/support/IntrusiveList.h>

// #if MATTER_DM_APPLICATION_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT > 0

using chip::EndpointId;
using chip::Span;
using chip::app::AttributeValueEncoder;
using chip::app::CommandResponseHelper;
using Application          = chip::app::Clusters::ApplicationLauncher::Structs::ApplicationStruct::Type;
using ApplicationEPType    = chip::app::Clusters::ApplicationLauncher::Structs::ApplicationEPStruct::Type;
using LauncherResponseType = chip::app::Clusters::ApplicationLauncher::Commands::LauncherResponse::Type;

namespace chip {
namespace app {
namespace Clusters {
namespace ApplicationLauncher {
namespace Chef {

class PlatformDelegate : public Delegate // This is for the "Casting Video Player" endpoint.
{
public:
    PlatformDelegate(EndpointId endpointId, const Span<const uint16_t> catalogList) :
        mEndpointId(endpointId), mCatalogList(catalogList)
    {
        mFeatureMapContentPlatform = true;
    }
    ~PlatformDelegate() = default;

    // Commands
    void HandleLaunchApp(CommandResponseHelper<LauncherResponseType> & helper, const ByteSpan & data,
                         const Application & application) override;
    void HandleStopApp(CommandResponseHelper<LauncherResponseType> & helper, const Application & application) override;
    void HandleHideApp(CommandResponseHelper<LauncherResponseType> & helper, const Application & application) override;

    // Attributes

    // this attribute should only be enabled for app platform instance
    CHIP_ERROR HandleGetCatalogList(app::AttributeValueEncoder & aEncoder) override;

    // this attribute should only be enabled for app platform instance
    CHIP_ERROR HandleGetCurrentApp(app::AttributeValueEncoder & aEncoder);

    EndpointId GetEndpointId() { return mEndpointId; }

private:
    EndpointId mEndpointId;
    // In a real app, you would track which app is currently in focus
    EndpointId mCurrentAppEndpoint = chip::kInvalidEndpointId;
    Span<const uint16_t> mCatalogList;
};

void AddApplicationLauncherPlatformDelegateForEndpoint(EndpointId endpoint);

class AppDelegate : public Delegate, public chip::IntrusiveListNodeBase<> // This is for the "Content App" endpoint.
{
public:
    AppDelegate(EndpointId endpointId) : mEndpointId(endpointId) {}
    ~AppDelegate() = default;

    // Commands
    void HandleLaunchApp(CommandResponseHelper<LauncherResponseType> & helper, const ByteSpan & data,
                         const Application & application) override;
    void HandleStopApp(CommandResponseHelper<LauncherResponseType> & helper, const Application & application) override;
    void HandleHideApp(CommandResponseHelper<LauncherResponseType> & helper, const Application & application) override;

    EndpointId GetEndpointId() { return mEndpointId; }

private:
    EndpointId mEndpointId;
    // TODO: Add an attribute for the app basic delegate.
};

void AddApplicationLauncherAppDelegateForEndpoint(EndpointId endpoint);

} // namespace Chef
} // namespace ApplicationLauncher
} // namespace Clusters
} // namespace app
} // namespace chip

// #endif
