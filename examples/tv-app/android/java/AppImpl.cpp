/*
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

/**
 * @file Contains Implementation of the ContentApp and the ContentAppPlatform.
 */

#include "AppImpl.h"

#include "ContentAppCommandDelegate.h"
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/CommandHandler.h>
#include <app/InteractionModelEngine.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <cstdio>
#include <inttypes.h>
#include <jni.h>
#include <lib/core/CHIPCore.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ZclString.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::AppPlatform;

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
ContentAppFactoryImpl gFactory;

ContentAppFactoryImpl * GetContentAppFactoryImpl()
{
    return &gFactory;
}

namespace chip {
namespace AppPlatform {

// BEGIN DYNAMIC ENDPOINTS
// =================================================================================

static const int kNameSize = 32;

// Current ZCL implementation of Struct uses a max-size array of 254 bytes
static const int kDescriptorAttributeArraySize = 254;

// Device types for dynamic endpoints: TODO Need a generated file from ZAP to define these!
// (taken from chip-devices.xml)
#define DEVICE_TYPE_CONTENT_APP 0x0024

// ---------------------------------------------------------------------------
//
// CONTENT APP ENDPOINT: contains the following clusters:
//   - Descriptor
//   - Application Basic
//   - Keypad Input
//   - Application Launcher
//   - Account Login
//   - Content Launcher
//   - Target Navigator
//   - Channel

// Declare Descriptor cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(descriptorAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_DEVICE_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0),     /* device list */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_SERVER_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0), /* server list */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_CLIENT_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0), /* client list */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_PARTS_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0),  /* parts list */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Application Basic information cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(applicationBasicAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_APPLICATION_VENDOR_NAME_ATTRIBUTE_ID, CHAR_STRING, kNameSize, 0), /* VendorName */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_APPLICATION_VENDOR_ID_ATTRIBUTE_ID, INT16U, 1, 0),            /* VendorID */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_APPLICATION_NAME_ATTRIBUTE_ID, CHAR_STRING, kNameSize, 0),    /* ApplicationName */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_APPLICATION_PRODUCT_ID_ATTRIBUTE_ID, INT16U, 1, 0),           /* ProductID */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_APPLICATION_STATUS_ATTRIBUTE_ID, INT8U, 1, 0),                /* ApplicationStatus */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_APPLICATION_VERSION_ATTRIBUTE_ID, CHAR_STRING, kNameSize, 0), /* ApplicationVersion */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_APPLICATION_ALLOWED_VENDOR_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize,
                              0), /* AllowedVendorList */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Keypad Input cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(keypadInputAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_FEATURE_MAP_SERVER_ATTRIBUTE_ID, BITMAP32, 4, 0), /* FeatureMap */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Application Launcher cluster attributes
// NOTE: Does not make sense for content app to be able to set the AP feature flag
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(applicationLauncherAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_APPLICATION_LAUNCHER_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0), /* catalog list */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_APPLICATION_LAUNCHER_CURRENT_APP_ATTRIBUTE_ID, STRUCT, 1, 0),                 /* current app */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Account Login cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(accountLoginAttrs)
DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Content Launcher cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(contentLauncherAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_CONTENT_LAUNCHER_ACCEPT_HEADER_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize,
                          0), /* accept header list */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_CONTENT_LAUNCHER_SUPPORTED_STREAMING_PROTOCOLS_ATTRIBUTE_ID, BITMAP32, 1,
                              0),                                                   /* streaming protocols */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_FEATURE_MAP_SERVER_ATTRIBUTE_ID, BITMAP32, 4, 0), /* FeatureMap */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Media Playback cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(mediaPlaybackAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_MEDIA_PLAYBACK_STATE_ATTRIBUTE_ID, ENUM8, 1, 0),                          /* current state */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_MEDIA_PLAYBACK_START_TIME_ATTRIBUTE_ID, EPOCH_US, 1, 0),              /* start time */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_MEDIA_PLAYBACK_DURATION_ATTRIBUTE_ID, INT64U, 1, 0),                  /* duration */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_MEDIA_PLAYBACK_PLAYBACK_POSITION_ATTRIBUTE_ID, STRUCT, 1, 0),         /* playback speed */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_MEDIA_PLAYBACK_PLAYBACK_SPEED_ATTRIBUTE_ID, SINGLE, 1, 0),            /* playback speed */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_MEDIA_PLAYBACK_PLAYBACK_SEEK_RANGE_END_ATTRIBUTE_ID, INT64U, 1, 0),   /* seek range end */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_MEDIA_PLAYBACK_PLAYBACK_SEEK_RANGE_START_ATTRIBUTE_ID, INT64U, 1, 0), /* seek range start */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_FEATURE_MAP_SERVER_ATTRIBUTE_ID, BITMAP32, 4, 0),                     /* FeatureMap */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Target Navigator cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(targetNavigatorAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_TARGET_NAVIGATOR_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0), /* target list */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_TARGET_NAVIGATOR_CURRENT_TARGET_ATTRIBUTE_ID, INT8U, 1, 0),               /* current target */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Channel cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(channelAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_CHANNEL_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0), /* channel list */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_CHANNEL_LINEUP_ATTRIBUTE_ID, STRUCT, 1, 0),                      /* lineup */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_CHANNEL_CURRENT_CHANNEL_ATTRIBUTE_ID, STRUCT, 1, 0),             /* current channel */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_FEATURE_MAP_SERVER_ATTRIBUTE_ID, BITMAP32, 4, 0),                /* FeatureMap */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

constexpr CommandId keypadInputIncomingCommands[] = {
    app::Clusters::KeypadInput::Commands::SendKey::Id,
    kInvalidCommandId,
};
constexpr CommandId keypadInputOutgoingCommands[] = {
    app::Clusters::KeypadInput::Commands::SendKeyResponse::Id,
    kInvalidCommandId,
};
constexpr CommandId applicationLauncherIncomingCommands[] = {
    app::Clusters::ApplicationLauncher::Commands::LaunchApp::Id,
    app::Clusters::ApplicationLauncher::Commands::StopApp::Id,
    app::Clusters::ApplicationLauncher::Commands::HideApp::Id,
    kInvalidCommandId,
};
constexpr CommandId applicationLauncherOutgoingCommands[] = {
    app::Clusters::ApplicationLauncher::Commands::LauncherResponse::Id,
    kInvalidCommandId,
};
constexpr CommandId accountLoginIncomingCommands[] = {
    app::Clusters::AccountLogin::Commands::GetSetupPIN::Id,
    app::Clusters::AccountLogin::Commands::Login::Id,
    app::Clusters::AccountLogin::Commands::Logout::Id,
    kInvalidCommandId,
};
constexpr CommandId accountLoginOutgoingCommands[] = {
    app::Clusters::AccountLogin::Commands::GetSetupPINResponse::Id,
    kInvalidCommandId,
};
// TODO: Sort out when the optional commands here should be listed.
constexpr CommandId contentLauncherIncomingCommands[] = {
    app::Clusters::ContentLauncher::Commands::LaunchContent::Id,
    app::Clusters::ContentLauncher::Commands::LaunchURL::Id,
    kInvalidCommandId,
};
constexpr CommandId contentLauncherOutgoingCommands[] = {
    app::Clusters::ContentLauncher::Commands::LaunchResponse::Id,
    kInvalidCommandId,
};
// TODO: Sort out when the optional commands here should be listed.
constexpr CommandId mediaPlaybackIncomingCommands[] = {
    app::Clusters::MediaPlayback::Commands::Play::Id,         app::Clusters::MediaPlayback::Commands::Pause::Id,
    app::Clusters::MediaPlayback::Commands::StopPlayback::Id, app::Clusters::MediaPlayback::Commands::StartOver::Id,
    app::Clusters::MediaPlayback::Commands::Previous::Id,     app::Clusters::MediaPlayback::Commands::Next::Id,
    app::Clusters::MediaPlayback::Commands::Rewind::Id,       app::Clusters::MediaPlayback::Commands::FastForward::Id,
    app::Clusters::MediaPlayback::Commands::SkipForward::Id,  app::Clusters::MediaPlayback::Commands::SkipBackward::Id,
    app::Clusters::MediaPlayback::Commands::Seek::Id,         kInvalidCommandId,
};
constexpr CommandId mediaPlaybackOutgoingCommands[] = {
    app::Clusters::MediaPlayback::Commands::PlaybackResponse::Id,
    kInvalidCommandId,
};
constexpr CommandId targetNavigatorIncomingCommands[] = {
    app::Clusters::TargetNavigator::Commands::NavigateTarget::Id,
    kInvalidCommandId,
};
constexpr CommandId targetNavigatorOutgoingCommands[] = {
    app::Clusters::TargetNavigator::Commands::NavigateTargetResponse::Id,
    kInvalidCommandId,
};
// TODO: Sort out when the optional commands here should be listed.
constexpr CommandId channelIncomingCommands[] = {
    app::Clusters::Channel::Commands::ChangeChannel::Id,
    app::Clusters::Channel::Commands::ChangeChannelByNumber::Id,
    app::Clusters::Channel::Commands::SkipChannel::Id,
    kInvalidCommandId,
};
constexpr CommandId channelOutgoingCommands[] = {
    app::Clusters::Channel::Commands::ChangeChannelResponse::Id,
    kInvalidCommandId,
};
// Declare Cluster List for Content App endpoint
DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(contentAppClusters)
DECLARE_DYNAMIC_CLUSTER(ZCL_DESCRIPTOR_CLUSTER_ID, descriptorAttrs, nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(ZCL_APPLICATION_BASIC_CLUSTER_ID, applicationBasicAttrs, nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(ZCL_KEYPAD_INPUT_CLUSTER_ID, keypadInputAttrs, keypadInputIncomingCommands,
                            keypadInputOutgoingCommands),
    DECLARE_DYNAMIC_CLUSTER(ZCL_APPLICATION_LAUNCHER_CLUSTER_ID, applicationLauncherAttrs, applicationLauncherIncomingCommands,
                            applicationLauncherOutgoingCommands),
    DECLARE_DYNAMIC_CLUSTER(ZCL_ACCOUNT_LOGIN_CLUSTER_ID, accountLoginAttrs, accountLoginIncomingCommands,
                            accountLoginOutgoingCommands),
    DECLARE_DYNAMIC_CLUSTER(ZCL_CONTENT_LAUNCH_CLUSTER_ID, contentLauncherAttrs, contentLauncherIncomingCommands,
                            contentLauncherOutgoingCommands),
    DECLARE_DYNAMIC_CLUSTER(ZCL_MEDIA_PLAYBACK_CLUSTER_ID, mediaPlaybackAttrs, mediaPlaybackIncomingCommands,
                            mediaPlaybackOutgoingCommands),
    DECLARE_DYNAMIC_CLUSTER(ZCL_TARGET_NAVIGATOR_CLUSTER_ID, targetNavigatorAttrs, targetNavigatorIncomingCommands,
                            targetNavigatorOutgoingCommands),
    DECLARE_DYNAMIC_CLUSTER(ZCL_CHANNEL_CLUSTER_ID, channelAttrs, channelIncomingCommands, channelOutgoingCommands),
    DECLARE_DYNAMIC_CLUSTER_LIST_END;

// Declare Content App endpoint
DECLARE_DYNAMIC_ENDPOINT(contentAppEndpoint, contentAppClusters);

namespace {

DataVersion gDataVersions[APP_LIBRARY_SIZE][ArraySize(contentAppClusters)];

EmberAfDeviceType gContentAppDeviceType[] = { { DEVICE_TYPE_CONTENT_APP, 1 } };

} // anonymous namespace

ContentAppFactoryImpl::ContentAppFactoryImpl() {}

uint16_t ContentAppFactoryImpl::GetPlatformCatalogVendorId()
{
    return kCatalogVendorId;
}

CHIP_ERROR ContentAppFactoryImpl::LookupCatalogVendorApp(uint16_t vendorId, uint16_t productId, CatalogVendorApp * destinationApp)
{
    std::string appId               = BuildAppId(vendorId);
    destinationApp->catalogVendorId = GetPlatformCatalogVendorId();
    Platform::CopyString(destinationApp->applicationId, sizeof(destinationApp->applicationId), appId.c_str());
    return CHIP_NO_ERROR;
}

CHIP_ERROR ContentAppFactoryImpl::ConvertToPlatformCatalogVendorApp(const CatalogVendorApp & sourceApp,
                                                                    CatalogVendorApp * destinationApp)
{
    destinationApp->catalogVendorId = GetPlatformCatalogVendorId();
    std::string appId(sourceApp.applicationId);
    if (appId == "applicationId")
    {
        // regression test case passes "applicationId", map this to our test suite app
        Platform::CopyString(destinationApp->applicationId, sizeof(destinationApp->applicationId), "1111");
    }
    else if (appId == "exampleid")
    {
        // cert test case passes "exampleid", map this to our test suite app
        Platform::CopyString(destinationApp->applicationId, sizeof(destinationApp->applicationId), "1");
    }
    else if (appId == "exampleString")
    {
        // cert test case passes "exampleString", map this to our test suite app
        Platform::CopyString(destinationApp->applicationId, sizeof(destinationApp->applicationId), "65521");
    }
    else
    {
        // for now, just return the applicationId passed in
        Platform::CopyString(destinationApp->applicationId, sizeof(destinationApp->applicationId), sourceApp.applicationId);
    }
    return CHIP_NO_ERROR;
}

ContentApp * ContentAppFactoryImpl::LoadContentApp(const CatalogVendorApp & vendorApp)
{
    ChipLogProgress(DeviceLayer, "ContentAppFactoryImpl: LoadContentAppByAppId catalogVendorId=%d applicationId=%s ",
                    vendorApp.catalogVendorId, vendorApp.applicationId);

    for (size_t i = 0; i < mContentApps.size(); ++i)
    {
        auto & app = mContentApps.at(i);

        ChipLogProgress(DeviceLayer, " Looking next=%s ", app->GetApplicationBasicDelegate()->GetCatalogVendorApp()->applicationId);
        if (app->GetApplicationBasicDelegate()->GetCatalogVendorApp()->Matches(vendorApp))
        {
            ContentAppPlatform::GetInstance().AddContentApp(app, &contentAppEndpoint, Span<DataVersion>(gDataVersions[i]),
                                                            Span<const EmberAfDeviceType>(gContentAppDeviceType));
            return app;
        }
    }
    ChipLogProgress(DeviceLayer, "LoadContentAppByAppId NOT FOUND catalogVendorId=%d applicationId=%s ", vendorApp.catalogVendorId,
                    vendorApp.applicationId);

    return nullptr;
}

EndpointId ContentAppFactoryImpl::AddContentApp(const char * szVendorName, uint16_t vendorId, const char * szApplicationName,
                                                uint16_t productId, const char * szApplicationVersion, jobject manager)
{
    DataVersion * dataVersionBuf = new DataVersion[ArraySize(contentAppClusters)];
    ContentAppImpl * app =
        new ContentAppImpl(szVendorName, vendorId, szApplicationName, productId, szApplicationVersion, "20202021", manager);
    EndpointId epId = ContentAppPlatform::GetInstance().AddContentApp(
        app, &contentAppEndpoint, Span<DataVersion>(dataVersionBuf, ArraySize(contentAppClusters)),
        Span<const EmberAfDeviceType>(gContentAppDeviceType));
    ChipLogProgress(DeviceLayer, "ContentAppFactoryImpl AddContentApp endpoint returned %d. Endpoint set %d", epId,
                    app->GetEndpointId());
    mContentApps.push_back(app);
    mDataVersions.push_back(dataVersionBuf);
    return epId;
}

EndpointId ContentAppFactoryImpl::AddContentApp(const char * szVendorName, uint16_t vendorId, const char * szApplicationName,
                                                uint16_t productId, const char * szApplicationVersion, jobject manager,
                                                EndpointId desiredEndpointId)
{
    DataVersion * dataVersionBuf = new DataVersion[ArraySize(contentAppClusters)];
    ContentAppImpl * app =
        new ContentAppImpl(szVendorName, vendorId, szApplicationName, productId, szApplicationVersion, "20202021", manager);
    EndpointId epId = ContentAppPlatform::GetInstance().AddContentApp(
        app, &contentAppEndpoint, Span<DataVersion>(dataVersionBuf, ArraySize(contentAppClusters)),
        Span<const EmberAfDeviceType>(gContentAppDeviceType), desiredEndpointId);
    ChipLogProgress(DeviceLayer, "ContentAppFactoryImpl AddContentApp endpoint returned %d. Endpoint set %d", epId,
                    app->GetEndpointId());
    mContentApps.push_back(app);
    mDataVersions.push_back(dataVersionBuf);
    return epId;
}

EndpointId ContentAppFactoryImpl::RemoveContentApp(EndpointId epId)
{
    for (size_t i = 0; i < mContentApps.size(); ++i)
    {
        auto & app = mContentApps.at(i);
        if (app->GetEndpointId() == epId)
        {
            ChipLogProgress(DeviceLayer, "ContentAppFactoryImpl RemoveContentApp endpointId %d", epId);
            EndpointId removedEndpointID = ContentAppPlatform::GetInstance().RemoveContentApp(app);
            // Only remove the app from the set of content apps if they were dynamically added and not part of the static list of
            // apps
            if (removedEndpointID != 0 && i > APP_LIBRARY_SIZE)
            {
                mContentApps.erase(mContentApps.begin() + static_cast<int>(i));
                DataVersion * dataVersionBuf = mDataVersions.at(i - APP_LIBRARY_SIZE);
                mDataVersions.erase(mDataVersions.begin() + static_cast<int>(i - APP_LIBRARY_SIZE));
                // deallocate memory for objects that were created when adding the content app dynamically.
                delete[] dataVersionBuf;
                delete app;
            }
            return removedEndpointID;
        }
    }
    return kInvalidEndpointId;
}

void ContentAppFactoryImpl::AddAdminVendorId(uint16_t vendorId)
{
    mAdminVendorIds.push_back(vendorId);
}

Access::Privilege ContentAppFactoryImpl::GetVendorPrivilege(uint16_t vendorId)
{
    for (size_t i = 0; i < mAdminVendorIds.size(); ++i)
    {
        auto & vendor = mAdminVendorIds.at(i);
        if (vendorId == vendor)
        {
            return Access::Privilege::kAdminister;
        }
    }
    return Access::Privilege::kOperate;
}

} // namespace AppPlatform
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

CHIP_ERROR InitVideoPlayerPlatform(jobject contentAppEndpointManager)
{
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ContentAppPlatform::GetInstance().SetupAppPlatform();
    ContentAppPlatform::GetInstance().SetContentAppFactory(&gFactory);

    ChipLogProgress(AppServer, "Starting registration of command handler delegates");
    for (size_t i = 0; i < ArraySize(contentAppClusters); i++)
    {
        ContentAppCommandDelegate * delegate =
            new ContentAppCommandDelegate(contentAppEndpointManager, contentAppClusters[i].clusterId);
        chip::app::InteractionModelEngine::GetInstance()->RegisterCommandHandler(delegate);
        ChipLogProgress(AppServer, "Registered command handler delegate for cluster %d", contentAppClusters[i].clusterId);
    }

#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

    // Disable last fixed endpoint, which is used as a placeholder for all of the
    // supported clusters so that ZAP will generated the requisite code.
    ChipLogDetail(DeviceLayer, "TV App: Disabling Fixed Content App Endpoints");
    emberAfEndpointEnableDisable(3, false);
    return CHIP_NO_ERROR;
}

EndpointId AddContentApp(const char * szVendorName, uint16_t vendorId, const char * szApplicationName, uint16_t productId,
                         const char * szApplicationVersion, jobject manager)
{
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ChipLogProgress(DeviceLayer, "AppImpl: AddContentApp vendorId=%d applicationName=%s ", vendorId, szApplicationName);
    return gFactory.AddContentApp(szVendorName, vendorId, szApplicationName, productId, szApplicationVersion, manager);
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    return kInvalidEndpointId;
}

EndpointId AddContentApp(const char * szVendorName, uint16_t vendorId, const char * szApplicationName, uint16_t productId,
                         const char * szApplicationVersion, EndpointId endpointId, jobject manager)
{
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ChipLogProgress(DeviceLayer, "AppImpl: AddContentApp vendorId=%d applicationName=%s ", vendorId, szApplicationName);
    return gFactory.AddContentApp(szVendorName, vendorId, szApplicationName, productId, szApplicationVersion, manager, endpointId);
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    return kInvalidEndpointId;
}

EndpointId RemoveContentApp(EndpointId epId)
{
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ChipLogProgress(DeviceLayer, "AppImpl: RemoveContentApp endpointId=%d ", epId);
    return gFactory.RemoveContentApp(epId);
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    return kInvalidEndpointId;
}
