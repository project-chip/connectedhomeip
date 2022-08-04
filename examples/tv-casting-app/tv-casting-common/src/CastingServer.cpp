/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "CastingServer.h"

using namespace chip;
using namespace chip::Controller;
using namespace chip::Credentials;
using namespace chip::app::Clusters::ContentLauncher::Commands;

CastingServer * CastingServer::castingServer_ = nullptr;

CastingServer::CastingServer()
{
#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
    // generate and set a random uniqueId for generating rotatingId
    uint8_t rotatingDeviceIdUniqueId[chip::DeviceLayer::ConfigurationManager::kRotatingDeviceIDUniqueIDLength];
    for (size_t i = 0; i < sizeof(rotatingDeviceIdUniqueId); i++)
    {
        rotatingDeviceIdUniqueId[i] = chip::Crypto::GetRandU8();
    }

    ByteSpan rotatingDeviceIdUniqueIdSpan(rotatingDeviceIdUniqueId);
    chip::DeviceLayer::ConfigurationMgr().SetRotatingDeviceIdUniqueId(rotatingDeviceIdUniqueIdSpan);
#endif // CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
}

CastingServer * CastingServer::GetInstance()
{
    if (castingServer_ == nullptr)
    {
        castingServer_ = new CastingServer();
    }
    return castingServer_;
}

void CastingServer::Init()
{
    if (mInited)
    {
        return;
    }

    // Initialize binding handlers
    ReturnOnFailure(InitBindingHandlers());

    // Add callback to send Content casting commands after commissioning completes
    ReturnOnFailure(DeviceLayer::PlatformMgrImpl().AddEventHandler(DeviceEventCallback, 0));

    mInited = true;
}

CHIP_ERROR CastingServer::InitBindingHandlers()
{
    auto & server = chip::Server::GetInstance();
    chip::BindingManager::GetInstance().Init(
        { &server.GetFabricTable(), server.GetCASESessionManager(), &server.GetPersistentStorage() });
    return CHIP_NO_ERROR;
}

CHIP_ERROR CastingServer::TargetVideoPlayerInfoInit(NodeId nodeId, FabricIndex fabricIndex)
{
    Init();
    return mTargetVideoPlayerInfo.Initialize(nodeId, fabricIndex);
}

CHIP_ERROR CastingServer::DiscoverCommissioners()
{
    // Send discover commissioners request
    return mCommissionableNodeController.DiscoverCommissioners(
        Dnssd::DiscoveryFilter(Dnssd::DiscoveryFilterType::kDeviceType, static_cast<uint16_t>(35)));
}

CHIP_ERROR CastingServer::OpenBasicCommissioningWindow(std::function<void(CHIP_ERROR)> commissioningCompleteCallback)
{
    mCommissioningCompleteCallback = commissioningCompleteCallback;
    return Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow(kCommissioningWindowTimeout);
}

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
CHIP_ERROR CastingServer::SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress commissioner)
{
    return Server::GetInstance().SendUserDirectedCommissioningRequest(commissioner);
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

const Dnssd::DiscoveredNodeData * CastingServer::GetDiscoveredCommissioner(int index)
{
    return mCommissionableNodeController.GetDiscoveredCommissioner(index);
}

void CastingServer::ReadServerClustersForNode(NodeId nodeId)
{
    ChipLogProgress(NotSpecified, "ReadServerClustersForNode nodeId=0x" ChipLogFormatX64, ChipLogValueX64(nodeId));
    for (const auto & binding : BindingTable::GetInstance())
    {
        ChipLogProgress(NotSpecified,
                        "Binding type=%d fab=%d nodeId=0x" ChipLogFormatX64
                        " groupId=%d local endpoint=%d remote endpoint=%d cluster=" ChipLogFormatMEI,
                        binding.type, binding.fabricIndex, ChipLogValueX64(binding.nodeId), binding.groupId, binding.local,
                        binding.remote, ChipLogValueMEI(binding.clusterId.ValueOr(0)));
        if (binding.type == EMBER_UNICAST_BINDING && nodeId == binding.nodeId)
        {
            if (!mTargetVideoPlayerInfo.HasEndpoint(binding.remote))
            {
                ReadServerClusters(binding.remote);
            }
            else
            {
                TargetEndpointInfo * endpointInfo = mTargetVideoPlayerInfo.GetEndpoint(binding.remote);
                if (endpointInfo != nullptr && endpointInfo->IsInitialized())
                {
                    endpointInfo->PrintInfo();
                }
            }
        }
    }
}

void CastingServer::ReadServerClusters(EndpointId endpointId)
{
    const OperationalDeviceProxy * deviceProxy = mTargetVideoPlayerInfo.GetOperationalDeviceProxy();
    if (deviceProxy == nullptr)
    {
        ChipLogError(AppServer, "Failed in getting an instance of DeviceProxy");
        return;
    }

    // GetOperationalDeviceProxy only passes us a deviceProxy if we can get a SessionHandle.
    chip::Controller::DescriptorCluster cluster(*deviceProxy->GetExchangeManager(), deviceProxy->GetSecureSession().Value(),
                                                endpointId);

    TargetEndpointInfo * endpointInfo = mTargetVideoPlayerInfo.GetOrAddEndpoint(endpointId);

    if (cluster.ReadAttribute<app::Clusters::Descriptor::Attributes::ServerList::TypeInfo>(
            endpointInfo, CastingServer::OnDescriptorReadSuccessResponse, CastingServer::OnDescriptorReadFailureResponse) !=
        CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Could not read Descriptor cluster ServerList");
    }

    ChipLogProgress(Controller, "Sent descriptor read for remote endpoint=%d", endpointId);
}

void CastingServer::OnDescriptorReadSuccessResponse(void * context, const app::DataModel::DecodableList<ClusterId> & responseList)
{
    TargetEndpointInfo * endpointInfo = static_cast<TargetEndpointInfo *>(context);

    ChipLogProgress(AppServer, "Descriptor: Default Success Response endpoint=%d", endpointInfo->GetEndpointId());

    auto iter = responseList.begin();
    while (iter.Next())
    {
        auto & clusterId = iter.GetValue();
        endpointInfo->AddCluster(clusterId);
    }
    // Always print the target info after handling descriptor read response
    // Even when we get nothing back for any reasons
    CastingServer::GetInstance()->mTargetVideoPlayerInfo.PrintInfo();
}

void CastingServer::OnDescriptorReadFailureResponse(void * context, CHIP_ERROR error)
{
    ChipLogError(AppServer, "Descriptor: Default Failure Response: %" CHIP_ERROR_FORMAT, error.Format());
}

[[deprecated("Use ContentLauncher_LaunchURL(..) instead")]] CHIP_ERROR
CastingServer::ContentLauncherLaunchURL(const char * contentUrl, const char * contentDisplayStr,
                                        std::function<void(CHIP_ERROR)> launchURLResponseCallback)
{
    return ContentLauncher_LaunchURL(contentUrl, contentDisplayStr,
                                     MakeOptional(chip::app::Clusters::ContentLauncher::Structs::BrandingInformation::Type()),
                                     launchURLResponseCallback);
}

void CastingServer::DeviceEventCallback(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    if (event->Type == DeviceLayer::DeviceEventType::kBindingsChangedViaCluster)
    {
        if (CastingServer::GetInstance()->GetTargetVideoPlayerInfo()->IsInitialized())
        {
            CastingServer::GetInstance()->ReadServerClustersForNode(
                CastingServer::GetInstance()->GetTargetVideoPlayerInfo()->GetNodeId());
        }
    }
    else if (event->Type == DeviceLayer::DeviceEventType::kCommissioningComplete)
    {
        CHIP_ERROR err = CastingServer::GetInstance()->GetTargetVideoPlayerInfo()->Initialize(
            event->CommissioningComplete.nodeId, event->CommissioningComplete.fabricIndex);

        CastingServer::GetInstance()->mCommissioningCompleteCallback(err);
    }
}

// given a fabric index, try to determine the video-player nodeId by searching the binding table
NodeId CastingServer::GetVideoPlayerNodeForFabricIndex(FabricIndex fabricIndex)
{
    for (const auto & binding : BindingTable::GetInstance())
    {
        ChipLogProgress(NotSpecified,
                        "Binding type=%d fab=%d nodeId=0x" ChipLogFormatX64
                        " groupId=%d local endpoint=%d remote endpoint=%d cluster=" ChipLogFormatMEI,
                        binding.type, binding.fabricIndex, ChipLogValueX64(binding.nodeId), binding.groupId, binding.local,
                        binding.remote, ChipLogValueMEI(binding.clusterId.ValueOr(0)));
        if (binding.type == EMBER_UNICAST_BINDING && fabricIndex == binding.fabricIndex)
        {
            ChipLogProgress(NotSpecified, "GetVideoPlayerNodeForFabricIndex nodeId=0x" ChipLogFormatX64,
                            ChipLogValueX64(binding.nodeId));
            return binding.nodeId;
        }
    }
    ChipLogProgress(NotSpecified, "GetVideoPlayerNodeForFabricIndex no bindings found for fabricIndex=%d", fabricIndex);
    return kUndefinedNodeId;
}

// given a nodeId, try to determine the video-player fabric index by searching the binding table
FabricIndex CastingServer::GetVideoPlayerFabricIndexForNode(NodeId nodeId)
{
    for (const auto & binding : BindingTable::GetInstance())
    {
        ChipLogProgress(NotSpecified,
                        "Binding type=%d fab=%d nodeId=0x" ChipLogFormatX64
                        " groupId=%d local endpoint=%d remote endpoint=%d cluster=" ChipLogFormatMEI,
                        binding.type, binding.fabricIndex, ChipLogValueX64(binding.nodeId), binding.groupId, binding.local,
                        binding.remote, ChipLogValueMEI(binding.clusterId.ValueOr(0)));
        if (binding.type == EMBER_UNICAST_BINDING && nodeId == binding.nodeId)
        {
            ChipLogProgress(NotSpecified, "GetVideoPlayerFabricIndexForNode fabricIndex=%d nodeId=0x" ChipLogFormatX64,
                            binding.fabricIndex, ChipLogValueX64(binding.nodeId));
            return binding.fabricIndex;
        }
    }
    ChipLogProgress(NotSpecified, "GetVideoPlayerFabricIndexForNode no bindings found for nodeId=0x" ChipLogFormatX64,
                    ChipLogValueX64(nodeId));
    return kUndefinedFabricIndex;
}

void CastingServer::SetDefaultFabricIndex()
{
    Init();

    // set fabric to be the first in the list
    for (const auto & fb : chip::Server::GetInstance().GetFabricTable())
    {
        FabricIndex fabricIndex = fb.GetFabricIndex();
        ChipLogError(AppServer, "Next Fabric index=%d", fabricIndex);
        if (!fb.IsInitialized())
        {
            ChipLogError(AppServer, " -- Not initialized");
            continue;
        }
        NodeId myNodeId = fb.GetNodeId();
        ChipLogProgress(NotSpecified,
                        "---- Current Fabric nodeId=0x" ChipLogFormatX64 " fabricId=0x" ChipLogFormatX64 " fabricIndex=%d",
                        ChipLogValueX64(myNodeId), ChipLogValueX64(fb.GetFabricId()), fabricIndex);

        NodeId videoPlayerNodeId = GetVideoPlayerNodeForFabricIndex(fabricIndex);
        if (videoPlayerNodeId == kUndefinedNodeId)
        {
            // could not determine video player nodeid for this fabric
            continue;
        }

        mTargetVideoPlayerInfo.Initialize(videoPlayerNodeId, fabricIndex);
        return;
    }
    ChipLogError(AppServer, " -- No initialized fabrics with video players");
}

CHIP_ERROR CastingServer::ContentLauncher_LaunchURL(
    const char * contentUrl, const char * contentDisplayStr,
    chip::Optional<chip::app::Clusters::ContentLauncher::Structs::BrandingInformation::Type> brandingInformation,
    std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mLaunchURLCommand.SetTarget(mTargetVideoPlayerInfo, kTvEndpoint));
    return mLaunchURLCommand.Invoke(contentUrl, contentDisplayStr, brandingInformation, responseCallback);
}

CHIP_ERROR CastingServer::ContentLauncher_LaunchContent(chip::app::Clusters::ContentLauncher::Structs::ContentSearch::Type search,
                                                        bool autoPlay, chip::Optional<chip::CharSpan> data,
                                                        std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mLaunchContentCommand.SetTarget(mTargetVideoPlayerInfo, kTvEndpoint));
    return mLaunchContentCommand.Invoke(search, autoPlay, data, responseCallback);
}

CHIP_ERROR CastingServer::LevelControl_Step(chip::app::Clusters::LevelControl::StepMode stepMode, uint8_t stepSize,
                                            uint16_t transitionTime, uint8_t optionMask, uint8_t optionOverride,
                                            std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mStepCommand.SetTarget(mTargetVideoPlayerInfo, kTvEndpoint));

    app::DataModel::Nullable<uint16_t> nullableTransitionTime;
    nullableTransitionTime.SetNonNull(transitionTime);

    return mStepCommand.Invoke(stepMode, stepSize, nullableTransitionTime, optionMask, optionOverride, responseCallback);
}

CHIP_ERROR CastingServer::LevelControl_MoveToLevel(uint8_t level, uint16_t transitionTime, uint8_t optionMask,
                                                   uint8_t optionOverride, std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mMoveToLevelCommand.SetTarget(mTargetVideoPlayerInfo, kTvEndpoint));

    app::DataModel::Nullable<uint16_t> nullableTransitionTime;
    nullableTransitionTime.SetNonNull(transitionTime);

    return mMoveToLevelCommand.Invoke(level, nullableTransitionTime, optionMask, optionOverride, responseCallback);
}

CHIP_ERROR CastingServer::MediaPlayback_Play(std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mPlayCommand.SetTarget(mTargetVideoPlayerInfo, kTvEndpoint));
    return mPlayCommand.Invoke(responseCallback);
}

CHIP_ERROR CastingServer::MediaPlayback_Pause(std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mPauseCommand.SetTarget(mTargetVideoPlayerInfo, kTvEndpoint));
    return mPauseCommand.Invoke(responseCallback);
}

CHIP_ERROR CastingServer::MediaPlayback_StopPlayback(std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mStopPlaybackCommand.SetTarget(mTargetVideoPlayerInfo, kTvEndpoint));
    return mStopPlaybackCommand.Invoke(responseCallback);
}

CHIP_ERROR CastingServer::MediaPlayback_Next(std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mNextCommand.SetTarget(mTargetVideoPlayerInfo, kTvEndpoint));
    return mNextCommand.Invoke(responseCallback);
}

CHIP_ERROR CastingServer::MediaPlayback_Seek(uint64_t position, std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mSeekCommand.SetTarget(mTargetVideoPlayerInfo, kTvEndpoint));
    return mSeekCommand.Invoke(position, responseCallback);
}

CHIP_ERROR CastingServer::MediaPlayback_SkipForward(uint64_t deltaPositionMilliseconds,
                                                    std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mSkipForwardCommand.SetTarget(mTargetVideoPlayerInfo, kTvEndpoint));
    return mSkipForwardCommand.Invoke(deltaPositionMilliseconds, responseCallback);
}

CHIP_ERROR CastingServer::MediaPlayback_SkipBackward(uint64_t deltaPositionMilliseconds,
                                                     std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mSkipBackwardCommand.SetTarget(mTargetVideoPlayerInfo, kTvEndpoint));
    return mSkipBackwardCommand.Invoke(deltaPositionMilliseconds, responseCallback);
}

CHIP_ERROR
CastingServer::ApplicationLauncher_LaunchApp(chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application,
                                             chip::Optional<chip::ByteSpan> data, std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mLaunchAppCommand.SetTarget(mTargetVideoPlayerInfo, kTvEndpoint));
    return mLaunchAppCommand.Invoke(application, data, responseCallback);
}

CHIP_ERROR
CastingServer::ApplicationLauncher_StopApp(chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application,
                                           std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mStopAppCommand.SetTarget(mTargetVideoPlayerInfo, kTvEndpoint));
    return mStopAppCommand.Invoke(application, responseCallback);
}

CHIP_ERROR
CastingServer::ApplicationLauncher_HideApp(chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application,
                                           std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mHideAppCommand.SetTarget(mTargetVideoPlayerInfo, kTvEndpoint));
    return mHideAppCommand.Invoke(application, responseCallback);
}

CHIP_ERROR CastingServer::TargetNavigator_NavigateTarget(const uint8_t target, const chip::Optional<chip::CharSpan> data,
                                                         std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mNavigateTargetCommand.SetTarget(mTargetVideoPlayerInfo, kTvEndpoint));
    return mNavigateTargetCommand.Invoke(target, data, responseCallback);
}

CHIP_ERROR CastingServer::KeypadInput_SendKey(const chip::app::Clusters::KeypadInput::CecKeyCode keyCode,
                                              std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mSendKeyCommand.SetTarget(mTargetVideoPlayerInfo, kTvEndpoint));
    return mSendKeyCommand.Invoke(keyCode, responseCallback);
}
