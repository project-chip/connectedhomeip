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

#include "application-basic/chef-application-basic-delegate.h"
#include <app/clusters/application-basic-server/application-basic-delegate.h>
#include <app/clusters/application-launcher-server/application-launcher-delegate.h>
#include <app/util/config.h>
#include <lib/support/IntrusiveList.h>

#if MATTER_DM_APPLICATION_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT > 0

namespace chip::app::Clusters::ApplicationLauncher::Chef {

class AppDelegate;

class PlatformDelegate : public Delegate // This is for the "Casting Video Player" endpoint.
{
public:
    PlatformDelegate(chip::EndpointId endpointId, const chip::Span<const uint16_t> catalogList) :
        mEndpointId(endpointId), mCatalogList(catalogList)
    {
        mFeatureMapContentPlatform = true;
    }
    ~PlatformDelegate() = default;

    // Commands
    void HandleLaunchApp(chip::app::CommandResponseHelper<ApplicationLauncher::Commands::LauncherResponse::Type> & helper,
                         const ByteSpan & data, const ApplicationLauncher::Structs::ApplicationStruct::Type & application) override;
    void HandleStopApp(chip::app::CommandResponseHelper<ApplicationLauncher::Commands::LauncherResponse::Type> & helper,
                       const ApplicationLauncher::Structs::ApplicationStruct::Type & application) override;
    void HandleHideApp(chip::app::CommandResponseHelper<ApplicationLauncher::Commands::LauncherResponse::Type> & helper,
                       const ApplicationLauncher::Structs::ApplicationStruct::Type & application) override;

    // Attributes

    // this attribute should only be enabled for app platform instance
    CHIP_ERROR HandleGetCatalogList(chip::app::AttributeValueEncoder & aEncoder) override;

    // this attribute should only be enabled for app platform instance
    CHIP_ERROR HandleGetCurrentApp(chip::app::AttributeValueEncoder & aEncoder);

    chip::EndpointId GetEndpointId() { return mEndpointId; }

    CHIP_ERROR AddAppDelegate(AppDelegate * delegate); // Adds a new app to the list of managed apps

    AppDelegate * FindAppDelegate(const ApplicationLauncher::Structs::ApplicationStruct::Type & application);

    void Register();

private:
    const chip::EndpointId mEndpointId;
    AppDelegate * mCurrentApp = nullptr; // track which app is currently in focus
    chip::Span<const uint16_t> mCatalogList;
    chip::IntrusiveList<AppDelegate> mAppDelegateList; // List of managed content apps
};

class AppDelegate : public Delegate, public chip::IntrusiveListNodeBase<> // This is for the "Content App" endpoints.
{
public:
    AppDelegate(chip::EndpointId endpointId, ApplicationBasic::Chef::ChefDelegate * appBasicDelegate) :
        mEndpointId(endpointId), mAppBasicDelegate(appBasicDelegate)
    {}
    ~AppDelegate() = default;

    // Commands
    void HandleLaunchApp(chip::app::CommandResponseHelper<ApplicationLauncher::Commands::LauncherResponse::Type> & helper,
                         const ByteSpan & data, const ApplicationLauncher::Structs::ApplicationStruct::Type & application) override;
    void HandleStopApp(chip::app::CommandResponseHelper<ApplicationLauncher::Commands::LauncherResponse::Type> & helper,
                       const ApplicationLauncher::Structs::ApplicationStruct::Type & application) override;
    void HandleHideApp(chip::app::CommandResponseHelper<ApplicationLauncher::Commands::LauncherResponse::Type> & helper,
                       const ApplicationLauncher::Structs::ApplicationStruct::Type & application) override;

    // Return error as this is not the platform instance
    CHIP_ERROR HandleGetCatalogList(chip::app::AttributeValueEncoder & aEncoder) override;

    chip::EndpointId GetEndpointId() { return mEndpointId; }

    void Register();

    bool Match(const ApplicationLauncher::Structs::ApplicationStruct::Type &
                   application); // Checks if the specified application matches with the current one

    ApplicationBasic::CatalogVendorApp * GetCatalogVendorApp() { return mAppBasicDelegate->GetCatalogVendorApp(); }

    void setPlatformDelegate(PlatformDelegate * platformDelegate) { mPlatformDelegate = platformDelegate; }

    void SetApplicationStatus(ApplicationBasic::ApplicationStatusEnum status) { mAppBasicDelegate->SetApplicationStatus(status); }

    ApplicationBasic::ApplicationStatusEnum GetApplicationStatus() const { return mAppBasicDelegate->GetApplicationStatus(); }

private:
    chip::EndpointId mEndpointId;
    ApplicationBasic::Chef::ChefDelegate * mAppBasicDelegate;
    PlatformDelegate * mPlatformDelegate = nullptr;
};

} // namespace chip::app::Clusters::ApplicationLauncher::Chef

#endif // MATTER_DM_APPLICATION_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT
