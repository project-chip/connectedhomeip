/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 * @brief Manages Content Apps
 */

#pragma once

#include <app/app-platform/ContentApp.h>
#include <app/app-platform/ContentAppPlatform.h>
#include <app/util/attribute-storage.h>
#include <functional>
#include <stdbool.h>
#include <stdint.h>

#include "account-login/AccountLoginManager.h"
#include "application-basic/ApplicationBasicManager.h"
#include "application-launcher/ApplicationLauncherManager.h"
#include "channel/ChannelManager.h"
#include "content-control/ContentController.h"
#include "content-launcher/ContentLauncherManager.h"
#include "keypad-input/KeypadInputManager.h"
#include "media-playback/MediaPlaybackManager.h"
#include "target-navigator/TargetNavigatorManager.h"

#include <app/clusters/account-login-server/account-login-delegate.h>
#include <app/clusters/application-basic-server/application-basic-delegate.h>
#include <app/clusters/application-launcher-server/application-launcher-delegate.h>
#include <app/clusters/channel-server/channel-delegate.h>
#include <app/clusters/content-launch-server/content-launch-delegate.h>
#include <app/clusters/keypad-input-server/keypad-input-delegate.h>
#include <app/clusters/media-playback-server/media-playback-delegate.h>
#include <app/clusters/target-navigator-server/target-navigator-delegate.h>

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

namespace chip {
namespace AppPlatform {

using AccountLoginDelegate        = app::Clusters::AccountLogin::Delegate;
using ApplicationBasicDelegate    = app::Clusters::ApplicationBasic::Delegate;
using ApplicationLauncherDelegate = app::Clusters::ApplicationLauncher::Delegate;
using ChannelDelegate             = app::Clusters::Channel::Delegate;
using ContentLauncherDelegate     = app::Clusters::ContentLauncher::Delegate;
using ContentControllerDelegate   = app::Clusters::ContentControl::Delegate;
using KeypadInputDelegate         = app::Clusters::KeypadInput::Delegate;
using MediaPlaybackDelegate       = app::Clusters::MediaPlayback::Delegate;
using TargetNavigatorDelegate     = app::Clusters::TargetNavigator::Delegate;
using SupportedProtocolsBitmap    = app::Clusters::ContentLauncher::SupportedProtocolsBitmap;

static const int kCatalogVendorId = CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID;

// for this platform, appid is just vendor id
#define BuildAppId(vid) std::to_string(vid).c_str()

class DLL_EXPORT ContentAppImpl : public ContentApp
{
public:
    ContentAppImpl(const char * szVendorName, uint16_t vendorId, const char * szApplicationName, uint16_t productId,
                   const char * szApplicationVersion, const char * setupPIN) :
        mApplicationBasicDelegate(kCatalogVendorId, BuildAppId(vendorId), szVendorName, vendorId, szApplicationName, productId,
                                  szApplicationVersion),
        mAccountLoginDelegate(setupPIN),
        mContentLauncherDelegate({ "image/*", "video/*" },
                                 to_underlying(SupportedProtocolsBitmap::kDash) | to_underlying(SupportedProtocolsBitmap::kHls)),
        mTargetNavigatorDelegate({ "home", "search", "info", "guide", "menu" }, 0){};
    virtual ~ContentAppImpl() {}

    AccountLoginDelegate * GetAccountLoginDelegate() override { return &mAccountLoginDelegate; };
    ApplicationBasicDelegate * GetApplicationBasicDelegate() override { return &mApplicationBasicDelegate; };
    ApplicationLauncherDelegate * GetApplicationLauncherDelegate() override { return &mApplicationLauncherDelegate; };
    ChannelDelegate * GetChannelDelegate() override { return &mChannelDelegate; };
    ContentLauncherDelegate * GetContentLauncherDelegate() override { return &mContentLauncherDelegate; };
    ContentControllerDelegate * GetContentControlDelegate() override { return &mContentControlDelegate; };
    KeypadInputDelegate * GetKeypadInputDelegate() override { return &mKeypadInputDelegate; };
    MediaPlaybackDelegate * GetMediaPlaybackDelegate() override { return &mMediaPlaybackDelegate; };
    TargetNavigatorDelegate * GetTargetNavigatorDelegate() override { return &mTargetNavigatorDelegate; };

protected:
    ApplicationBasicManager mApplicationBasicDelegate;
    AccountLoginManager mAccountLoginDelegate;
    ApplicationLauncherManager mApplicationLauncherDelegate;
    ChannelManager mChannelDelegate;
    ContentLauncherManager mContentLauncherDelegate;
    ContentControlManager mContentControlDelegate;
    KeypadInputManager mKeypadInputDelegate;
    MediaPlaybackManager mMediaPlaybackDelegate;
    TargetNavigatorManager mTargetNavigatorDelegate;
};

class DLL_EXPORT ContentAppFactoryImpl : public ContentAppFactory
{
#define APP_LIBRARY_SIZE 4
public:
    ContentAppFactoryImpl();
    virtual ~ContentAppFactoryImpl() {}

    // Lookup CatalogVendor App for this client (vendor id/product id client)
    // and then write it to destinationApp
    // return error if not found
    CHIP_ERROR LookupCatalogVendorApp(uint16_t vendorId, uint16_t productId, CatalogVendorApp * destinationApp) override;

    // Lookup ContentApp for this catalog id / app id and load it
    ContentApp * LoadContentApp(const CatalogVendorApp & vendorApp) override;

    // Gets the catalog vendor ID used by this platform
    uint16_t GetPlatformCatalogVendorId() override;

    // Converts application (any catalog) into the platform's catalog Vendor
    // and then writes it to destinationApp
    CHIP_ERROR ConvertToPlatformCatalogVendorApp(const CatalogVendorApp & sourceApp, CatalogVendorApp * destinationApp) override;

    // Get the privilege this vendorId should have on endpoints 1, 2, and content app endpoints
    // In the case of casting video clients, this should usually be Access::Privilege::kOperate
    // and for voice agents, this may be Access::Privilege::kAdminister
    // When a vendor has admin privileges, it will get access to all clusters on ep1
    Access::Privilege GetVendorPrivilege(uint16_t vendorId) override;

    // Get the cluster list this vendorId/productId should have on static endpoints such as ep1 for casting video clients.
    // When a vendor has admin privileges, it will get access to all clusters on ep1
    std::list<ClusterId> GetAllowedClusterListForStaticEndpoint(EndpointId endpointId, uint16_t vendorId,
                                                                uint16_t productId) override;

    void AddAdminVendorId(uint16_t vendorId);

protected:
    ContentAppImpl mContentApps[APP_LIBRARY_SIZE] = {
        ContentAppImpl("Vendor1", 1, "exampleid", 11, "Version1", "34567890"),
        ContentAppImpl("Vendor2", 65521, "exampleString", 32768, "Version2", "20202021"),
        ContentAppImpl("Vendor3", 9050, "App3", 22, "Version3", "20202021"),
        ContentAppImpl("TestSuiteVendor", 1111, "applicationId", 22, "v2", "20202021")
    };

    std::vector<uint16_t> mAdminVendorIds{};
};

} // namespace AppPlatform
} // namespace chip

chip::AppPlatform::ContentAppFactoryImpl * GetContentAppFactoryImpl();

#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

CHIP_ERROR AppTvInit(void);
