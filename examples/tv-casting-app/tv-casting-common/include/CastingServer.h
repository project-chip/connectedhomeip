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

#pragma once

#include <app/server/Server.h>
#include <controller/CHIPCommissionableNodeController.h>
#include <functional>
#include <zap-generated/CHIPClusters.h>

#include "TargetEndpointInfo.h"
#include "TargetVideoPlayerInfo.h"

constexpr chip::System::Clock::Seconds16 kCommissioningWindowTimeout = chip::System::Clock::Seconds16(3 * 60);
constexpr chip::EndpointId kTvEndpoint                               = 1;

/**
 * @brief Represents a TV Casting server that can get the casting app commissioned
 *  and then have it send TV Casting/Media related commands. This is to be instantiated
 *  as a singleton and is to be used across Linux, Android and iOS.
 */
class CastingServer
{
public:
    CastingServer(CastingServer & other) = delete;
    void operator=(const CastingServer &) = delete;
    static CastingServer * GetInstance();

    void InitServer(std::function<CHIP_ERROR()> commissioningCompleteCallback);

    CHIP_ERROR DiscoverCommissioners();
    const chip::Dnssd::DiscoveredNodeData * GetDiscoveredCommissioner(int index);
    CHIP_ERROR OpenBasicCommissioningWindow();
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    CHIP_ERROR SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress commissioner);
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

    CHIP_ERROR InitBindingHandlers();
    TargetVideoPlayerInfo * GetTargetVideoPlayerInfo() { return &mTargetVideoPlayerInfo; }
    CHIP_ERROR TargetVideoPlayerInfoInit(chip::NodeId nodeId, chip::FabricIndex fabricIndex,
                                         std::function<CHIP_ERROR()> commissioningCompleteCallback);
    void ReadServerClusters(chip::EndpointId endpointId);
    void ReadServerClustersForNode(chip::NodeId nodeId);

    static void OnDescriptorReadSuccessResponse(void * context,
                                                const chip::app::DataModel::DecodableList<chip::ClusterId> & responseList);
    static void OnDescriptorReadFailureResponse(void * context, CHIP_ERROR error);
    CHIP_ERROR ContentLauncherLaunchURL(const char * contentUrl, const char * contentDisplayStr);
    static void OnContentLauncherSuccessResponse(
        void * context, const chip::app::Clusters::ContentLauncher::Commands::LaunchResponse::DecodableType & response);
    static void OnContentLauncherFailureResponse(void * context, CHIP_ERROR error);
    static void DeviceEventCallback(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);

    chip::NodeId GetVideoPlayerNodeForFabricIndex(chip::FabricIndex fabricIndex);
    chip::FabricIndex GetVideoPlayerFabricIndexForNode(chip::NodeId nodeId);
    void PrintBindings();
    chip::FabricIndex CurrentFabricIndex() { return mTargetVideoPlayerInfo.GetFabricIndex(); }
    void SetDefaultFabricIndex(std::function<CHIP_ERROR()> commissioningCompleteCallback);

private:
    static CastingServer * castingServer_;
    CastingServer();

    bool mInited = false;
    TargetVideoPlayerInfo mTargetVideoPlayerInfo;
    chip::Controller::CommissionableNodeController mCommissionableNodeController;
    std::function<CHIP_ERROR()> mCommissioningCompleteCallback;
};
