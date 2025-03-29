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

#include "ContentAppAttributeDelegate.h"
#include "ContentAppCommandDelegate.h"
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/CommandHandler.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/reporting/reporting.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <app/util/endpoint-config-api.h>
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
#include <platform/DeviceInstanceInfoProvider.h>

#include <string>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::AppPlatform;
using namespace chip::DeviceLayer;

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
DECLARE_DYNAMIC_ATTRIBUTE(Descriptor::Attributes::DeviceTypeList::Id, ARRAY, kDescriptorAttributeArraySize, 0), /* device list */
    DECLARE_DYNAMIC_ATTRIBUTE(Descriptor::Attributes::ServerList::Id, ARRAY, kDescriptorAttributeArraySize, 0), /* server list */
    DECLARE_DYNAMIC_ATTRIBUTE(Descriptor::Attributes::ClientList::Id, ARRAY, kDescriptorAttributeArraySize, 0), /* client list */
    DECLARE_DYNAMIC_ATTRIBUTE(Descriptor::Attributes::PartsList::Id, ARRAY, kDescriptorAttributeArraySize, 0),  /* parts list */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Application Basic information cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(applicationBasicAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ApplicationBasic::Attributes::VendorName::Id, CHAR_STRING, kNameSize, 0),          /* VendorName */
    DECLARE_DYNAMIC_ATTRIBUTE(ApplicationBasic::Attributes::VendorID::Id, INT16U, 1, 0),                     /* VendorID */
    DECLARE_DYNAMIC_ATTRIBUTE(ApplicationBasic::Attributes::ApplicationName::Id, CHAR_STRING, kNameSize, 0), /* ApplicationName */
    DECLARE_DYNAMIC_ATTRIBUTE(ApplicationBasic::Attributes::ProductID::Id, INT16U, 1, 0),                    /* ProductID */
    DECLARE_DYNAMIC_ATTRIBUTE(ApplicationBasic::Attributes::Status::Id, INT8U, 1, 0),                        /* ApplicationStatus */
    DECLARE_DYNAMIC_ATTRIBUTE(ApplicationBasic::Attributes::ApplicationVersion::Id, CHAR_STRING, kNameSize,
                              0), /* ApplicationVersion */
    DECLARE_DYNAMIC_ATTRIBUTE(ApplicationBasic::Attributes::AllowedVendorList::Id, ARRAY, kDescriptorAttributeArraySize,
                              0), /* AllowedVendorList */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Keypad Input cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(keypadInputAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(KeypadInput::Attributes::FeatureMap::Id, BITMAP32, 4, 0), /* FeatureMap */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Application Launcher cluster attributes
// NOTE: Does not make sense for content app to be able to set the AP feature flag
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(applicationLauncherAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ApplicationLauncher::Attributes::CatalogList::Id, ARRAY, kDescriptorAttributeArraySize,
                          0),                                                                 /* catalog list */
    DECLARE_DYNAMIC_ATTRIBUTE(ApplicationLauncher::Attributes::CurrentApp::Id, STRUCT, 1, 0), /* current app */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Account Login cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(accountLoginAttrs)
DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Content Launcher cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(contentLauncherAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ContentLauncher::Attributes::AcceptHeader::Id, ARRAY, kDescriptorAttributeArraySize,
                          0), /* accept header list */
    DECLARE_DYNAMIC_ATTRIBUTE(ContentLauncher::Attributes::SupportedStreamingProtocols::Id, BITMAP32, 1,
                              0),                                                           /* streaming protocols */
    DECLARE_DYNAMIC_ATTRIBUTE(ContentLauncher::Attributes::FeatureMap::Id, BITMAP32, 4, 0), /* FeatureMap */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Media Playback cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(mediaPlaybackAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(MediaPlayback::Attributes::CurrentState::Id, ENUM8, 1, 0),         /* current state */
    DECLARE_DYNAMIC_ATTRIBUTE(MediaPlayback::Attributes::StartTime::Id, EPOCH_US, 1, 0),     /* start time */
    DECLARE_DYNAMIC_ATTRIBUTE(MediaPlayback::Attributes::Duration::Id, INT64U, 1, 0),        /* duration */
    DECLARE_DYNAMIC_ATTRIBUTE(MediaPlayback::Attributes::SampledPosition::Id, STRUCT, 1, 0), /* SampledPosition */
    DECLARE_DYNAMIC_ATTRIBUTE(MediaPlayback::Attributes::PlaybackSpeed::Id, SINGLE, 1, 0),   /* playback speed */
    DECLARE_DYNAMIC_ATTRIBUTE(MediaPlayback::Attributes::SeekRangeEnd::Id, INT64U, 1, 0),    /* seek range end */
    DECLARE_DYNAMIC_ATTRIBUTE(MediaPlayback::Attributes::SeekRangeStart::Id, INT64U, 1, 0),  /* seek range start */
    DECLARE_DYNAMIC_ATTRIBUTE(MediaPlayback::Attributes::FeatureMap::Id, BITMAP32, 4, 0),    /* FeatureMap */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Target Navigator cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(targetNavigatorAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(TargetNavigator::Attributes::TargetList::Id, ARRAY, kDescriptorAttributeArraySize, 0), /* target list */
    DECLARE_DYNAMIC_ATTRIBUTE(TargetNavigator::Attributes::CurrentTarget::Id, INT8U, 1, 0), /* current target */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Channel cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(channelAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(Channel::Attributes::ChannelList::Id, ARRAY, kDescriptorAttributeArraySize, 0), /* channel list */
    DECLARE_DYNAMIC_ATTRIBUTE(Channel::Attributes::Lineup::Id, STRUCT, 1, 0),                             /* lineup */
    DECLARE_DYNAMIC_ATTRIBUTE(Channel::Attributes::CurrentChannel::Id, STRUCT, 1, 0),                     /* current channel */
    DECLARE_DYNAMIC_ATTRIBUTE(Channel::Attributes::FeatureMap::Id, BITMAP32, 4, 0),                       /* FeatureMap */
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
    app::Clusters::ContentLauncher::Commands::LauncherResponse::Id,
    kInvalidCommandId,
};
// TODO: Sort out when the optional commands here should be listed.
constexpr CommandId mediaPlaybackIncomingCommands[] = {
    app::Clusters::MediaPlayback::Commands::Play::Id,        app::Clusters::MediaPlayback::Commands::Pause::Id,
    app::Clusters::MediaPlayback::Commands::Stop::Id,        app::Clusters::MediaPlayback::Commands::StartOver::Id,
    app::Clusters::MediaPlayback::Commands::Previous::Id,    app::Clusters::MediaPlayback::Commands::Next::Id,
    app::Clusters::MediaPlayback::Commands::Rewind::Id,      app::Clusters::MediaPlayback::Commands::FastForward::Id,
    app::Clusters::MediaPlayback::Commands::SkipForward::Id, app::Clusters::MediaPlayback::Commands::SkipBackward::Id,
    app::Clusters::MediaPlayback::Commands::Seek::Id,        kInvalidCommandId,
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
DECLARE_DYNAMIC_CLUSTER(app::Clusters::Descriptor::Id, descriptorAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(app::Clusters::ApplicationBasic::Id, applicationBasicAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(app::Clusters::KeypadInput::Id, keypadInputAttrs, ZAP_CLUSTER_MASK(SERVER), keypadInputIncomingCommands,
                            keypadInputOutgoingCommands),
    DECLARE_DYNAMIC_CLUSTER(app::Clusters::ApplicationLauncher::Id, applicationLauncherAttrs, ZAP_CLUSTER_MASK(SERVER),
                            applicationLauncherIncomingCommands, applicationLauncherOutgoingCommands),
    DECLARE_DYNAMIC_CLUSTER(app::Clusters::AccountLogin::Id, accountLoginAttrs, ZAP_CLUSTER_MASK(SERVER),
                            accountLoginIncomingCommands, accountLoginOutgoingCommands),
    DECLARE_DYNAMIC_CLUSTER(app::Clusters::ContentLauncher::Id, contentLauncherAttrs, ZAP_CLUSTER_MASK(SERVER),
                            contentLauncherIncomingCommands, contentLauncherOutgoingCommands),
    DECLARE_DYNAMIC_CLUSTER(app::Clusters::MediaPlayback::Id, mediaPlaybackAttrs, ZAP_CLUSTER_MASK(SERVER),
                            mediaPlaybackIncomingCommands, mediaPlaybackOutgoingCommands),
    DECLARE_DYNAMIC_CLUSTER(app::Clusters::TargetNavigator::Id, targetNavigatorAttrs, ZAP_CLUSTER_MASK(SERVER),
                            targetNavigatorIncomingCommands, targetNavigatorOutgoingCommands),
    DECLARE_DYNAMIC_CLUSTER(app::Clusters::Channel::Id, channelAttrs, ZAP_CLUSTER_MASK(SERVER), channelIncomingCommands,
                            channelOutgoingCommands),
    DECLARE_DYNAMIC_CLUSTER_LIST_END;

// Declare Content App endpoint
DECLARE_DYNAMIC_ENDPOINT(contentAppEndpoint, contentAppClusters);

namespace {

DataVersion gDataVersions[APP_LIBRARY_SIZE][MATTER_ARRAY_SIZE(contentAppClusters)];

EmberAfDeviceType gContentAppDeviceType[] = { { DEVICE_TYPE_CONTENT_APP, 1 } };

std::vector<SupportedCluster> make_default_supported_clusters()
{
    return std::vector<ContentApp::SupportedCluster>{ { Descriptor::Id },      { ApplicationBasic::Id },
                                                      { KeypadInput::Id },     { ApplicationLauncher::Id },
                                                      { AccountLogin::Id },    { ContentLauncher::Id },
                                                      { TargetNavigator::Id }, { Channel::Id } };
}

} // anonymous namespace

ContentAppFactoryImpl::ContentAppFactoryImpl() :
    mContentApps{ new ContentAppImpl("Vendor1", 1, "exampleid", 11, "Version1", "20202021", make_default_supported_clusters(),
                                     nullptr, nullptr),
                  new ContentAppImpl("Vendor2", 65521, "exampleString", 32768, "Version2", "20202021",
                                     make_default_supported_clusters(), nullptr, nullptr),
                  new ContentAppImpl("Vendor3", 9050, "App3", 22, "Version3", "20202021", make_default_supported_clusters(),
                                     nullptr, nullptr),
                  new ContentAppImpl("TestSuiteVendor", 1111, "applicationId", 22, "v2", "20202021",
                                     make_default_supported_clusters(), nullptr, nullptr) }
{}

uint16_t ContentAppFactoryImpl::GetPlatformCatalogVendorId()
{
    return kCatalogVendorId;
}

void ContentAppFactoryImpl::setContentAppAttributeDelegate(ContentAppAttributeDelegate * attributeDelegate)
{
    mAttributeDelegate = attributeDelegate;
}

void ContentAppFactoryImpl::setContentAppCommandDelegate(ContentAppCommandDelegate * commandDelegate)
{
    mCommandDelegate = commandDelegate;
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
            // need to think about loading apk here?
            ContentAppPlatform::GetInstance().AddContentApp(app, &contentAppEndpoint, Span<DataVersion>(gDataVersions[i]),
                                                            Span<const EmberAfDeviceType>(gContentAppDeviceType));
            return app;
        }
    }
    ChipLogProgress(DeviceLayer, "LoadContentAppByAppId NOT FOUND catalogVendorId=%d applicationId=%s ", vendorApp.catalogVendorId,
                    vendorApp.applicationId);

    return nullptr;
}

class DevicePairedCommand : public Controller::DevicePairingDelegate
{
public:
    struct CallbackContext
    {
        uint16_t vendorId;
        uint16_t productId;
        chip::NodeId nodeId;

        CallbackContext(uint16_t vId, uint16_t pId, chip::NodeId nId) : vendorId(vId), productId(pId), nodeId(nId) {}
    };
    DevicePairedCommand(uint16_t vendorId, uint16_t productId, chip::NodeId nodeId) :
        mOnDeviceConnectedCallback(OnDeviceConnectedFn, this), mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
    {
        mContext = std::make_shared<CallbackContext>(vendorId, productId, nodeId);
    }

    static void OnDeviceConnectedFn(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                    const chip::SessionHandle & sessionHandle)
    {
        auto * pairingCommand = static_cast<DevicePairedCommand *>(context);
        auto cbContext        = pairingCommand->mContext;

        if (pairingCommand)
        {
            ChipLogProgress(DeviceLayer,
                            "OnDeviceConnectedFn - Updating ACL for node id: " ChipLogFormatX64
                            " and vendor id: %d and product id: %d",
                            ChipLogValueX64(cbContext->nodeId), cbContext->vendorId, cbContext->productId);

            GetCommissionerDiscoveryController()->CommissioningSucceeded(cbContext->vendorId, cbContext->productId,
                                                                         cbContext->nodeId, exchangeMgr, sessionHandle);
        }
    }

    static void OnDeviceConnectionFailureFn(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
    {
        auto * pairingCommand = static_cast<DevicePairedCommand *>(context);
        auto cbContext        = pairingCommand->mContext;

        if (pairingCommand)
        {
            ChipLogProgress(DeviceLayer,
                            "OnDeviceConnectionFailureFn - Not updating ACL for node id: " ChipLogFormatX64
                            " and vendor id: %d and product id: %d",
                            ChipLogValueX64(cbContext->nodeId), cbContext->vendorId, cbContext->productId);
            // TODO: Remove Node Id
        }
    }

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
    std::shared_ptr<CallbackContext> mContext;
};

void refreshConnectedClientsAcl(uint16_t vendorId, uint16_t productId, ContentAppImpl * app)
{

    std::set<NodeId> nodeIds = ContentAppPlatform::GetInstance().GetNodeIdsForContentApp(vendorId, productId);

    for (const auto & allowedVendor : app->GetApplicationBasicDelegate()->GetAllowedVendorList())
    {
        std::set<NodeId> tempNodeIds = ContentAppPlatform::GetInstance().GetNodeIdsForAllowedVendorId(allowedVendor);

        nodeIds.insert(tempNodeIds.begin(), tempNodeIds.end());
    }

    for (const auto & nodeId : nodeIds)
    {

        ChipLogProgress(DeviceLayer,
                        "Creating Pairing Command with node id: " ChipLogFormatX64 " and vendor id: %d and product id: %d",
                        ChipLogValueX64(nodeId), vendorId, productId);

        std::shared_ptr<DevicePairedCommand> pairingCommand = std::make_shared<DevicePairedCommand>(vendorId, productId, nodeId);

        GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                    &pairingCommand->mOnDeviceConnectionFailureCallback);
    }
}

EndpointId ContentAppFactoryImpl::AddContentApp(const char * szVendorName, uint16_t vendorId, const char * szApplicationName,
                                                uint16_t productId, const char * szApplicationVersion,
                                                std::vector<SupportedCluster> supportedClusters, jobject manager)
{
    DataVersion * dataVersionBuf = new DataVersion[MATTER_ARRAY_SIZE(contentAppClusters)];
    ContentAppImpl * app = new ContentAppImpl(szVendorName, vendorId, szApplicationName, productId, szApplicationVersion, "",
                                              std::move(supportedClusters), mAttributeDelegate, mCommandDelegate);
    EndpointId epId      = ContentAppPlatform::GetInstance().AddContentApp(
        app, &contentAppEndpoint, Span<DataVersion>(dataVersionBuf, MATTER_ARRAY_SIZE(contentAppClusters)),
        Span<const EmberAfDeviceType>(gContentAppDeviceType));
    ChipLogProgress(DeviceLayer, "ContentAppFactoryImpl AddContentApp endpoint returned %d. Endpoint set %d", epId,
                    app->GetEndpointId());
    mContentApps.push_back(app);
    mDataVersions.push_back(dataVersionBuf);

    refreshConnectedClientsAcl(vendorId, productId, app);

    return epId;
}

EndpointId ContentAppFactoryImpl::AddContentApp(const char * szVendorName, uint16_t vendorId, const char * szApplicationName,
                                                uint16_t productId, const char * szApplicationVersion,
                                                std::vector<SupportedCluster> supportedClusters, EndpointId desiredEndpointId,
                                                jobject manager)
{
    DataVersion * dataVersionBuf = new DataVersion[MATTER_ARRAY_SIZE(contentAppClusters)];
    ContentAppImpl * app = new ContentAppImpl(szVendorName, vendorId, szApplicationName, productId, szApplicationVersion, "",
                                              std::move(supportedClusters), mAttributeDelegate, mCommandDelegate);
    EndpointId epId      = ContentAppPlatform::GetInstance().AddContentApp(
        app, &contentAppEndpoint, Span<DataVersion>(dataVersionBuf, MATTER_ARRAY_SIZE(contentAppClusters)),
        Span<const EmberAfDeviceType>(gContentAppDeviceType), desiredEndpointId);
    ChipLogProgress(DeviceLayer, "ContentAppFactoryImpl AddContentApp endpoint returned %d. Endpoint set %d", epId,
                    app->GetEndpointId());
    mContentApps.push_back(app);
    mDataVersions.push_back(dataVersionBuf);

    refreshConnectedClientsAcl(vendorId, productId, app);

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

void ContentAppFactoryImpl::LogInstalledApps()
{
    for (auto & contentApp : mContentApps)
    {
        ChipLogProgress(DeviceLayer, "Content app vid=%d pid=%d id=%s is on ep=%d",
                        contentApp->GetApplicationBasicDelegate()->HandleGetVendorId(),
                        contentApp->GetApplicationBasicDelegate()->HandleGetProductId(),
                        contentApp->GetApplicationBasicDelegate()->GetCatalogVendorApp()->GetApplicationId(),
                        contentApp->GetEndpointId());
    }
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

std::list<ClusterId> ContentAppFactoryImpl::GetAllowedClusterListForStaticEndpoint(EndpointId endpointId, uint16_t vendorId,
                                                                                   uint16_t productId)
{
    if (endpointId == kLocalVideoPlayerEndpointId)
    {
        if (GetVendorPrivilege(vendorId) == Access::Privilege::kAdminister)
        {
            ChipLogProgress(DeviceLayer,
                            "ContentAppFactoryImpl GetAllowedClusterListForStaticEndpoint priviledged vendor accessible clusters "
                            "being returned.");
            return { chip::app::Clusters::Descriptor::Id,
                     chip::app::Clusters::OnOff::Id,
                     chip::app::Clusters::WakeOnLan::Id,
                     chip::app::Clusters::MediaPlayback::Id,
                     chip::app::Clusters::LowPower::Id,
                     chip::app::Clusters::KeypadInput::Id,
                     chip::app::Clusters::ContentLauncher::Id,
                     chip::app::Clusters::AudioOutput::Id,
                     chip::app::Clusters::ApplicationLauncher::Id,
                     chip::app::Clusters::Messages::Id };
        }
        ChipLogProgress(
            DeviceLayer,
            "ContentAppFactoryImpl GetAllowedClusterListForStaticEndpoint operator vendor accessible clusters being returned.");
        return { chip::app::Clusters::Descriptor::Id,      chip::app::Clusters::OnOff::Id,
                 chip::app::Clusters::WakeOnLan::Id,       chip::app::Clusters::MediaPlayback::Id,
                 chip::app::Clusters::LowPower::Id,        chip::app::Clusters::KeypadInput::Id,
                 chip::app::Clusters::ContentLauncher::Id, chip::app::Clusters::AudioOutput::Id,
                 chip::app::Clusters::Messages::Id };
    }
    return {};
}

} // namespace AppPlatform
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

CHIP_ERROR InitVideoPlayerPlatform(jobject contentAppEndpointManager)
{
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ContentAppPlatform::GetInstance().SetupAppPlatform();
    ContentAppPlatform::GetInstance().SetContentAppFactory(&gFactory);
    gFactory.setContentAppAttributeDelegate(new ContentAppAttributeDelegate(contentAppEndpointManager));
    gFactory.setContentAppCommandDelegate(new ContentAppCommandDelegate(contentAppEndpointManager));

    ChipLogProgress(AppServer, "Starting registration of command handler delegates");
    for (size_t i = 0; i < MATTER_ARRAY_SIZE(contentAppClusters); i++)
    {
        ContentAppCommandDelegate * delegate =
            new ContentAppCommandDelegate(contentAppEndpointManager, contentAppClusters[i].clusterId);
        app::CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(delegate);
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
                         const char * szApplicationVersion, std::vector<SupportedCluster> supportedClusters, jobject manager)
{
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ChipLogProgress(DeviceLayer, "AppImpl: AddContentApp vendorId=%d applicationName=%s ", vendorId, szApplicationName);
    return gFactory.AddContentApp(szVendorName, vendorId, szApplicationName, productId, szApplicationVersion,
                                  std::move(supportedClusters), manager);
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    return kInvalidEndpointId;
}

EndpointId AddContentApp(const char * szVendorName, uint16_t vendorId, const char * szApplicationName, uint16_t productId,
                         const char * szApplicationVersion, std::vector<SupportedCluster> supportedClusters, EndpointId endpointId,
                         jobject manager)
{
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ChipLogProgress(DeviceLayer, "AppImpl: AddContentApp vendorId=%d applicationName=%s ", vendorId, szApplicationName);
    return gFactory.AddContentApp(szVendorName, vendorId, szApplicationName, productId, szApplicationVersion,
                                  std::move(supportedClusters), endpointId, manager);
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

void ReportAttributeChange(EndpointId epId, chip::ClusterId clusterId, chip::AttributeId attributeId)
{
    MatterReportingAttributeChangeCallback(epId, clusterId, attributeId);
}

void AddSelfVendorAsAdmin()
{
    uint16_t value;
    if (DeviceLayer::GetDeviceInstanceInfoProvider()->GetVendorId(value) != CHIP_NO_ERROR)
    {
        ChipLogDetail(Discovery, "AppImpl addSelfVendorAsAdmin Vendor ID not known");
    }
    else
    {
        gFactory.AddAdminVendorId(value);
    }
}
