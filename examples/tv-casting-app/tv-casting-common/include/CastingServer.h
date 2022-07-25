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

#include "ApplicationLauncher.h"
#include "ContentLauncher.h"
#include "KeypadInput.h"
#include "LevelControl.h"
#include "MediaPlayback.h"
#include "TargetEndpointInfo.h"
#include "TargetNavigator.h"
#include "TargetVideoPlayerInfo.h"

#include <app/server/Server.h>
#include <controller/CHIPCommissionableNodeController.h>
#include <functional>
#include <zap-generated/CHIPClusters.h>

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

    void Init();

    CHIP_ERROR DiscoverCommissioners();
    const chip::Dnssd::DiscoveredNodeData * GetDiscoveredCommissioner(int index);
    CHIP_ERROR OpenBasicCommissioningWindow(std::function<void(CHIP_ERROR)> commissioningCompleteCallback);
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    CHIP_ERROR SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress commissioner);
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

    TargetVideoPlayerInfo * GetTargetVideoPlayerInfo() { return &mTargetVideoPlayerInfo; }
    CHIP_ERROR TargetVideoPlayerInfoInit(chip::NodeId nodeId, chip::FabricIndex fabricIndex);
    void ReadServerClusters(chip::EndpointId endpointId);
    void ReadServerClustersForNode(chip::NodeId nodeId);
    static void OnDescriptorReadSuccessResponse(void * context,
                                                const chip::app::DataModel::DecodableList<chip::ClusterId> & responseList);
    static void OnDescriptorReadFailureResponse(void * context, CHIP_ERROR error);

    [[deprecated("Use ContentLauncher_LaunchURL(..) instead")]] CHIP_ERROR
    ContentLauncherLaunchURL(const char * contentUrl, const char * contentDisplayStr,
                             std::function<void(CHIP_ERROR)> launchURLResponseCallback);

    chip::NodeId GetVideoPlayerNodeForFabricIndex(chip::FabricIndex fabricIndex);
    chip::FabricIndex GetVideoPlayerFabricIndexForNode(chip::NodeId nodeId);
    chip::FabricIndex CurrentFabricIndex() { return mTargetVideoPlayerInfo.GetFabricIndex(); }
    void SetDefaultFabricIndex();

    CHIP_ERROR ContentLauncher_LaunchURL(
        const char * contentUrl, const char * contentDisplayStr,
        chip::Optional<chip::app::Clusters::ContentLauncher::Structs::BrandingInformation::Type> brandingInformation,
        std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR ContentLauncher_LaunchContent(chip::app::Clusters::ContentLauncher::Structs::ContentSearch::Type search,
                                             bool autoPlay, chip::Optional<chip::CharSpan> data,
                                             std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR LevelControl_Step(chip::app::Clusters::LevelControl::StepMode stepMode, uint8_t stepSize, uint16_t transitionTime,
                                 uint8_t optionMask, uint8_t optionOverride, std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR LevelControl_MoveToLevel(uint8_t level, uint16_t transitionTime, uint8_t optionMask, uint8_t optionOverride,
                                        std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR MediaPlayback_Play(std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR MediaPlayback_Pause(std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR MediaPlayback_StopPlayback(std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR MediaPlayback_Next(std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR MediaPlayback_Seek(uint64_t position, std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR MediaPlayback_SkipForward(uint64_t deltaPositionMilliseconds, std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR MediaPlayback_SkipBackward(uint64_t deltaPositionMilliseconds, std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR ApplicationLauncher_LaunchApp(chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application,
                                             chip::Optional<chip::ByteSpan> data, std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR ApplicationLauncher_StopApp(chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application,
                                           std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR ApplicationLauncher_HideApp(chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application,
                                           std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR TargetNavigator_NavigateTarget(const uint8_t target, const chip::Optional<chip::CharSpan> data,
                                              std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR KeypadInput_SendKey(const chip::app::Clusters::KeypadInput::CecKeyCode keyCode,
                                   std::function<void(CHIP_ERROR)> responseCallback);

private:
    CHIP_ERROR InitBindingHandlers();
    static void DeviceEventCallback(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);

    static CastingServer * castingServer_;
    CastingServer();

    bool mInited = false;
    TargetVideoPlayerInfo mTargetVideoPlayerInfo;
    chip::Controller::CommissionableNodeController mCommissionableNodeController;
    std::function<void(CHIP_ERROR)> mLaunchURLResponseCallback;
    std::function<void(CHIP_ERROR)> mCommissioningCompleteCallback;

    /**
     * @brief Content Launcher cluster
     */
    LaunchURLCommand mLaunchURLCommand;
    LaunchContentCommand mLaunchContentCommand;

    /**
     * @brief Level Control cluster
     */
    StepCommand mStepCommand;
    MoveToLevelCommand mMoveToLevelCommand;

    /**
     * @brief Media Playback cluster
     */
    PlayCommand mPlayCommand;
    PauseCommand mPauseCommand;
    StopPlaybackCommand mStopPlaybackCommand;
    NextCommand mNextCommand;
    SeekCommand mSeekCommand;
    SkipForwardCommand mSkipForwardCommand;
    SkipBackwardCommand mSkipBackwardCommand;

    /**
     * @brief Application Launcher cluster
     */
    LaunchAppCommand mLaunchAppCommand;
    StopAppCommand mStopAppCommand;
    HideAppCommand mHideAppCommand;

    /**
     * @brief Target Navigator cluster
     */
    NavigateTargetCommand mNavigateTargetCommand;

    /**
     * @brief Keypad Input cluster
     */
    SendKeyCommand mSendKeyCommand;
};
