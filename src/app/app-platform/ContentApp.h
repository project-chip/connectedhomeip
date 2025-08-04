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
 * @brief classes relating to Content App of the Video Player.
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/account-login-server/account-login-delegate.h>
#include <app/clusters/application-basic-server/application-basic-delegate.h>
#include <app/clusters/application-launcher-server/application-launcher-delegate.h>
#include <app/clusters/channel-server/channel-delegate.h>
#include <app/clusters/content-control-server/content-control-delegate.h>
#include <app/clusters/content-launch-server/content-launch-delegate.h>
#include <app/clusters/keypad-input-server/keypad-input-delegate.h>
#include <app/clusters/media-playback-server/media-playback-delegate.h>
#include <app/clusters/target-navigator-server/target-navigator-delegate.h>
#include <app/util/attribute-storage.h>
#include <controller/CHIPDeviceController.h>
#include <lib/core/DataModelTypes.h>
#include <protocols/interaction_model/StatusCode.h>

#include <string>

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

inline constexpr uint8_t kMaxClientNodes = 8;

class ContentAppClientCommandSender
{
public:
    ContentAppClientCommandSender() :
        mOnDeviceConnectedCallback(OnDeviceConnectedFn, this), mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
    {}

    bool IsBusy() const { return mIsBusy; }
    CHIP_ERROR SendContentAppMessage(chip::Controller::DeviceCommissioner * commissioner, chip::NodeId destinationId,
                                     chip::EndpointId endPointId, char * data, char * encodingHint);

protected:
    CHIP_ERROR SendMessage(chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle);

    void Cleanup();

private:
    static void OnDeviceConnectedFn(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                    const chip::SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailureFn(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error);

    using ContentAppMessageResponseDecodableType =
        chip::app::Clusters::ContentAppObserver::Commands::ContentAppMessageResponse::DecodableType;

    static void OnCommandResponse(void * context, const ContentAppMessageResponseDecodableType & response);
    static void OnCommandFailure(void * context, CHIP_ERROR error);

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;

    bool mIsBusy                 = false;
    chip::NodeId mDestinationId  = 0;
    chip::EndpointId mEndPointId = 0;

    std::string mData;
    std::string mEncodingHint;
};

class DLL_EXPORT ContentApp
{
public:
    struct SupportedCluster
    {
        chip::ClusterId mClusterIdentifier{ kInvalidClusterId };
        uint32_t mFeatures{ 0 };
        std::vector<CommandId> mOptionalCommandIdentifiers;
        std::vector<AttributeId> mOptionalAttributesIdentifiers;

        SupportedCluster(ClusterId clusterId, uint32_t features, const std::vector<CommandId> & commandIds,
                         const std::vector<AttributeId> & attributeIds) :
            mClusterIdentifier{ clusterId },
            mFeatures{ features }, mOptionalCommandIdentifiers{ commandIds }, mOptionalAttributesIdentifiers{ attributeIds }
        {}

        SupportedCluster(ClusterId clusterId) : mClusterIdentifier{ clusterId } {}
    };

    ContentApp(std::vector<SupportedCluster> supportedClusters) : mSupportedClusters{ supportedClusters } {}

    virtual ~ContentApp() = default;

    inline void SetEndpointId(EndpointId id) { mEndpointId = id; };
    inline EndpointId GetEndpointId() { return mEndpointId; };

    const std::vector<SupportedCluster> & GetSupportedClusters() const { return mSupportedClusters; };
    bool HasSupportedCluster(ClusterId clusterId) const;

    virtual AccountLoginDelegate * GetAccountLoginDelegate()               = 0;
    virtual ApplicationBasicDelegate * GetApplicationBasicDelegate()       = 0;
    virtual ApplicationLauncherDelegate * GetApplicationLauncherDelegate() = 0;
    virtual ChannelDelegate * GetChannelDelegate()                         = 0;
    virtual ContentLauncherDelegate * GetContentLauncherDelegate()         = 0;
    virtual ContentControlDelegate * GetContentControlDelegate()           = 0;
    virtual KeypadInputDelegate * GetKeypadInputDelegate()                 = 0;
    virtual MediaPlaybackDelegate * GetMediaPlaybackDelegate()             = 0;
    virtual TargetNavigatorDelegate * GetTargetNavigatorDelegate()         = 0;

    Protocols::InteractionModel::Status HandleReadAttribute(ClusterId clusterId, AttributeId attributeId, uint8_t * buffer,
                                                            uint16_t maxReadLength);
    Protocols::InteractionModel::Status HandleWriteAttribute(ClusterId clusterId, AttributeId attributeId, uint8_t * buffer);

    // returns true only if new node is added. If node was added previously, then false is returned.
    bool AddClientNode(NodeId clientNodeId);
    uint8_t GetClientNodeCount() const { return mClientNodeCount; }
    NodeId GetClientNode(uint8_t index) const { return mClientNodes[index]; }

    void SendAppObserverCommand(chip::Controller::DeviceCommissioner * commissioner, NodeId clientNodeId, char * data,
                                char * encodingHint);

protected:
    EndpointId mEndpointId = 0;
    std::vector<SupportedCluster> mSupportedClusters;

    uint8_t mClientNodeCount     = 0;
    uint8_t mNextClientNodeIndex = 0;
    NodeId mClientNodes[kMaxClientNodes];

    ContentAppClientCommandSender mContentAppClientCommandSender;
};

} // namespace AppPlatform
} // namespace chip
