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
#include <jni.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/JniReferences.h>
#include <stdbool.h>
#include <stdint.h>
#include <vector>

#include "../include/account-login/AccountLoginManager.h"
#include "../include/application-basic/ApplicationBasicManager.h"
#include "../include/content-control/ContentController.h"
#include "../include/content-launcher/AppContentLauncherManager.h"
#include "../include/media-playback/AppMediaPlaybackManager.h"
#include "../include/target-navigator/TargetNavigatorManager.h"
#include "ChannelManager.h"
#include "CommissionerMain.h"
#include "ContentAppAttributeDelegate.h"
#include "ContentAppCommandDelegate.h"
#include "KeypadInputManager.h"
#include "application-launcher/ApplicationLauncherManager.h"
#include <app/clusters/account-login-server/account-login-delegate.h>
#include <app/clusters/application-basic-server/application-basic-delegate.h>
#include <app/clusters/application-launcher-server/application-launcher-delegate.h>
#include <app/clusters/channel-server/channel-delegate.h>
#include <app/clusters/content-control-server/content-control-delegate.h>
#include <app/clusters/content-launch-server/content-launch-delegate.h>
#include <app/clusters/keypad-input-server/keypad-input-delegate.h>
#include <app/clusters/media-playback-server/media-playback-delegate.h>
#include <app/clusters/target-navigator-server/target-navigator-delegate.h>

CHIP_ERROR InitVideoPlayerPlatform(jobject contentAppEndpointManager);
EndpointId AddContentApp(const char * szVendorName, uint16_t vendorId, const char * szApplicationName, uint16_t productId,
                         const char * szApplicationVersion,
                         std::vector<chip::AppPlatform::ContentApp::SupportedCluster> supportedClusters, jobject manager);
EndpointId AddContentApp(const char * szVendorName, uint16_t vendorId, const char * szApplicationName, uint16_t productId,
                         const char * szApplicationVersion,
                         std::vector<chip::AppPlatform::ContentApp::SupportedCluster> supportedClusters, EndpointId endpointId,
                         jobject manager);
EndpointId RemoveContentApp(EndpointId epId);
void ReportAttributeChange(EndpointId epId, chip::ClusterId clusterId, chip::AttributeId attributeId);

void AddSelfVendorAsAdmin();

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

namespace chip {
namespace AppPlatform {

using AccountLoginDelegate        = app::Clusters::AccountLogin::Delegate;
using ApplicationBasicDelegate    = app::Clusters::ApplicationBasic::Delegate;
using ApplicationLauncherDelegate = app::Clusters::ApplicationLauncher::Delegate;
using ChannelDelegate             = app::Clusters::Channel::Delegate;
using ContentLauncherDelegate     = app::Clusters::ContentLauncher::Delegate;
using ContentControlDelegate      = app::Clusters::ContentControl::Delegate;
using KeypadInputDelegate         = app::Clusters::KeypadInput::Delegate;
using MediaPlaybackDelegate       = app::Clusters::MediaPlayback::Delegate;
using TargetNavigatorDelegate     = app::Clusters::TargetNavigator::Delegate;
using SupportedProtocolsBitmap    = app::Clusters::ContentLauncher::SupportedProtocolsBitmap;
using ContentAppAttributeDelegate = chip::AppPlatform::ContentAppAttributeDelegate;
using ContentAppCommandDelegate   = chip::AppPlatform::ContentAppCommandDelegate;
using SupportedCluster            = chip::AppPlatform::ContentApp::SupportedCluster;

static const int kCatalogVendorId = CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID;

// for this platform, appid is just vendor id
#define BuildAppId(vid) std::to_string(vid).c_str()

class DLL_EXPORT ContentAppImpl : public ContentApp
{
public:
    ContentAppImpl(const char * szVendorName, uint16_t vendorId, const char * szApplicationName, uint16_t productId,
                   const char * szApplicationVersion, const char * setupPIN, std::vector<SupportedCluster> supportedClusters,
                   ContentAppAttributeDelegate * attributeDelegate, ContentAppCommandDelegate * commandDelegate) :
        ContentApp{ supportedClusters },
        mApplicationBasicDelegate(kCatalogVendorId, BuildAppId(vendorId), szVendorName, vendorId, szApplicationName, productId,
                                  szApplicationVersion),
        mAccountLoginDelegate(commandDelegate, setupPIN),
        mContentLauncherDelegate(attributeDelegate, { "image/*", "video/*" },
                                 to_underlying(SupportedProtocolsBitmap::kDash) | to_underlying(SupportedProtocolsBitmap::kHls)),
        mMediaPlaybackDelegate(attributeDelegate),
        mTargetNavigatorDelegate(attributeDelegate, { "home", "search", "info", "guide", "menu" }, 0){};
    virtual ~ContentAppImpl() {}

    AccountLoginDelegate * GetAccountLoginDelegate() override
    {
        mAccountLoginDelegate.SetEndpointId(GetEndpointId());
        return &mAccountLoginDelegate;
    };
    ApplicationBasicDelegate * GetApplicationBasicDelegate() override { return &mApplicationBasicDelegate; };
    ApplicationLauncherDelegate * GetApplicationLauncherDelegate() override { return &mApplicationLauncherDelegate; };
    ChannelDelegate * GetChannelDelegate() override { return &mChannelDelegate; };
    ContentLauncherDelegate * GetContentLauncherDelegate() override
    {
        mContentLauncherDelegate.SetEndpointId(GetEndpointId());
        return &mContentLauncherDelegate;
    };
    ContentControlDelegate * GetContentControlDelegate() override
    {
        mContentControlDelegate.SetEndpointId(GetEndpointId());
        return &mContentControlDelegate;
    };
    KeypadInputDelegate * GetKeypadInputDelegate() override { return &mKeypadInputDelegate; };
    MediaPlaybackDelegate * GetMediaPlaybackDelegate() override
    {
        mMediaPlaybackDelegate.SetEndpointId(GetEndpointId());
        return &mMediaPlaybackDelegate;
    };
    TargetNavigatorDelegate * GetTargetNavigatorDelegate() override
    {
        mTargetNavigatorDelegate.SetEndpointId(GetEndpointId());
        return &mTargetNavigatorDelegate;
    };

protected:
    ApplicationBasicManager mApplicationBasicDelegate;
    AccountLoginManager mAccountLoginDelegate;
    ApplicationLauncherManager mApplicationLauncherDelegate;
    ChannelManager mChannelDelegate;
    ContentController mContentControlDelegate;
    AppContentLauncherManager mContentLauncherDelegate;
    KeypadInputManager mKeypadInputDelegate;
    AppMediaPlaybackManager mMediaPlaybackDelegate;
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

    EndpointId AddContentApp(const char * szVendorName, uint16_t vendorId, const char * szApplicationName, uint16_t productId,
                             const char * szApplicationVersion, std::vector<SupportedCluster> supportedClusters, jobject manager);

    EndpointId AddContentApp(const char * szVendorName, uint16_t vendorId, const char * szApplicationName, uint16_t productId,
                             const char * szApplicationVersion, std::vector<SupportedCluster> supportedClusters,
                             EndpointId desiredEndpointId, jobject manager);

    EndpointId RemoveContentApp(EndpointId epId);

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

    void setContentAppAttributeDelegate(ContentAppAttributeDelegate * attributeDelegate);

    void setContentAppCommandDelegate(ContentAppCommandDelegate * commandDelegate);

    void LogInstalledApps();

protected:
    // TODO: Update to use unique_ptr instead of raw pointers
    std::vector<ContentAppImpl *> mContentApps;
    std::vector<DataVersion *> mDataVersions{};
    std::vector<uint16_t> mAdminVendorIds{};

private:
    ContentAppAttributeDelegate * mAttributeDelegate;
    ContentAppCommandDelegate * mCommandDelegate;
};

} // namespace AppPlatform
} // namespace chip

chip::AppPlatform::ContentAppFactoryImpl * GetContentAppFactoryImpl();

#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
