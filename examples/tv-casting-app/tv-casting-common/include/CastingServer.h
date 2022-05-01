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
#include <zap-generated/CHIPClusters.h>

#include "TargetEndpointInfo.h"
#include "TargetVideoPlayerInfo.h"

using namespace chip;
using namespace chip::Controller;
using namespace chip::Credentials;
using namespace chip::app::Clusters::ContentLauncher::Commands;

constexpr System::Clock::Seconds16 kCommissioningWindowTimeout = System::Clock::Seconds16(3 * 60);
constexpr EndpointId kTvEndpoint                               = 1;

/**
 * @brief Represents a TV Casting server that can get the casting app commissioned
 *  and then have it send TV Casting/Media related commands. This is to be instantiated
 *  as a singleton and is to be used across Linux, Android and iOS.
 */
class CastingServer
{
public:
    CastingServer(CastingServer & other)  = delete;
    void operator=(const CastingServer &) = delete;
    static CastingServer * GetInstance();

    void InitServer();

    CHIP_ERROR DiscoverCommissioners();
    const Dnssd::DiscoveredNodeData * GetDiscoveredCommissioner(int index);
    CHIP_ERROR OpenBasicCommissioningWindow();
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    CHIP_ERROR SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress commissioner);
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

    CHIP_ERROR InitBindingHandlers();
    TargetVideoPlayerInfo * GetTargetVideoPlayerInfo()
    {
        return &mTargetVideoPlayerInfo;
    }
    CHIP_ERROR TargetVideoPlayerInfoInit(chip::NodeId nodeId, chip::FabricIndex fabricIndex);
    void ReadServerClusters(EndpointId endpointId);
    void ReadServerClustersForNode(chip::NodeId nodeId);
    static void OnDescriptorReadSuccessResponse(void * context, const app::DataModel::DecodableList<ClusterId> & responseList);
    static void OnDescriptorReadFailureResponse(void * context, CHIP_ERROR error);
    CHIP_ERROR ContentLauncherLaunchURL(const char * contentUrl, const char * contentDisplayStr);
    static void OnContentLauncherSuccessResponse(void * context, const LaunchResponse::DecodableType & response);
    static void OnContentLauncherFailureResponse(void * context, CHIP_ERROR error);
    static void DeviceEventCallback(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg);

    NodeId GetVideoPlayerNodeForFabricIndex(FabricIndex fabricIndex);
    FabricIndex GetVideoPlayerFabricIndexForNode(NodeId nodeId);
    void PrintBindings();
    FabricIndex CurrentFabricIndex()
    {
        return mTargetVideoPlayerInfo.GetFabricIndex();
    }
    void SetDefaultFabricIndex();

private:
    static CastingServer * castingServer_;
    CastingServer() {}

    bool mInited = false;
    TargetVideoPlayerInfo mTargetVideoPlayerInfo;
    CommissionableNodeController mCommissionableNodeController;
};
