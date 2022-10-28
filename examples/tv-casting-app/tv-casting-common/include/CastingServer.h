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

#include "AppParams.h"
#include "ApplicationBasic.h"
#include "ApplicationLauncher.h"
#include "Channel.h"
#include "ContentLauncher.h"
#include "KeypadInput.h"
#include "LevelControl.h"
#include "MediaPlayback.h"
#include "PersistenceManager.h"
#include "TargetEndpointInfo.h"
#include "TargetNavigator.h"
#include "TargetVideoPlayerInfo.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/server/Server.h>
#include <controller/CHIPCommissionableNodeController.h>
#include <functional>
#include <tv-casting-app/zap-generated/CHIPClientCallbacks.h>
#include <zap-generated/CHIPClusters.h>

constexpr chip::System::Clock::Seconds16 kCommissioningWindowTimeout = chip::System::Clock::Seconds16(3 * 60);

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

    CHIP_ERROR Init(AppParams * AppParams = nullptr);

    CHIP_ERROR DiscoverCommissioners();
    const chip::Dnssd::DiscoveredNodeData *
    GetDiscoveredCommissioner(int index, chip::Optional<TargetVideoPlayerInfo *> & outAssociatedConnectableVideoPlayer);
    CHIP_ERROR OpenBasicCommissioningWindow(std::function<void(CHIP_ERROR)> commissioningCompleteCallback,
                                            std::function<void(TargetVideoPlayerInfo *)> onConnectionSuccess,
                                            std::function<void(CHIP_ERROR)> onConnectionFailure,
                                            std::function<void(TargetEndpointInfo *)> onNewOrUpdatedEndpoint);

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    CHIP_ERROR SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress commissioner);
    CHIP_ERROR SendUserDirectedCommissioningRequest(chip::Dnssd::DiscoveredNodeData * selectedCommissioner);
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

    TargetVideoPlayerInfo * GetActiveTargetVideoPlayer() { return &mActiveTargetVideoPlayerInfo; }

    CHIP_ERROR TargetVideoPlayerInfoInit(chip::NodeId nodeId, chip::FabricIndex fabricIndex,
                                         std::function<void(TargetVideoPlayerInfo *)> onConnectionSuccess,
                                         std::function<void(CHIP_ERROR)> onConnectionFailure,
                                         std::function<void(TargetEndpointInfo *)> onNewOrUpdatedEndpoint);
    void ReadServerClustersForNode(chip::NodeId nodeId);
    static void OnDescriptorReadSuccessResponse(void * context,
                                                const chip::app::DataModel::DecodableList<chip::ClusterId> & responseList);
    static void OnDescriptorReadFailureResponse(void * context, CHIP_ERROR error);

    [[deprecated("Use ContentLauncher_LaunchURL(..) instead")]] CHIP_ERROR
    ContentLauncherLaunchURL(TargetEndpointInfo * endpoint, const char * contentUrl, const char * contentDisplayStr,
                             std::function<void(CHIP_ERROR)> launchURLResponseCallback);

    chip::NodeId GetVideoPlayerNodeForFabricIndex(chip::FabricIndex fabricIndex);
    chip::FabricIndex GetVideoPlayerFabricIndexForNode(chip::NodeId nodeId);
    chip::FabricIndex CurrentFabricIndex() { return mActiveTargetVideoPlayerInfo.GetFabricIndex(); }
    void SetDefaultFabricIndex(std::function<void(TargetVideoPlayerInfo *)> onConnectionSuccess,
                               std::function<void(CHIP_ERROR)> onConnectionFailure,
                               std::function<void(TargetEndpointInfo *)> onNewOrUpdatedEndpoint);
    TargetVideoPlayerInfo * ReadCachedTargetVideoPlayerInfos();
    CHIP_ERROR VerifyOrEstablishConnection(TargetVideoPlayerInfo & targetVideoPlayerInfo,
                                           std::function<void(TargetVideoPlayerInfo *)> onConnectionSuccess,
                                           std::function<void(CHIP_ERROR)> onConnectionFailure,
                                           std::function<void(TargetEndpointInfo *)> onNewOrUpdatedEndpoint);

    CHIP_ERROR PurgeVideoPlayerCache();

    /**
     * Tears down all active subscriptions.
     */
    void ShutdownAllSubscriptions();

    /**
     *  Mark any open session with the currently connected Video player as expired.
     */
    void Disconnect();

    /**
     * @brief Content Launcher cluster
     */
    CHIP_ERROR ContentLauncher_LaunchURL(
        TargetEndpointInfo * endpoint, const char * contentUrl, const char * contentDisplayStr,
        chip::Optional<chip::app::Clusters::ContentLauncher::Structs::BrandingInformation::Type> brandingInformation,
        std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR ContentLauncher_LaunchContent(TargetEndpointInfo * endpoint,
                                             chip::app::Clusters::ContentLauncher::Structs::ContentSearch::Type search,
                                             bool autoPlay, chip::Optional<chip::CharSpan> data,
                                             std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR
    ContentLauncher_SubscribeToAcceptHeader(
        TargetEndpointInfo * endpoint, void * context,
        chip::Controller::ReadResponseSuccessCallback<
            chip::app::Clusters::ContentLauncher::Attributes::AcceptHeader::TypeInfo::DecodableArgType>
            successFn,
        chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
        chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished);
    CHIP_ERROR
    ContentLauncher_SubscribeToSupportedStreamingProtocols(
        TargetEndpointInfo * endpoint, void * context,
        chip::Controller::ReadResponseSuccessCallback<
            chip::app::Clusters::ContentLauncher::Attributes::SupportedStreamingProtocols::TypeInfo::DecodableArgType>
            successFn,
        chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
        chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished);

    /**
     * @brief Level Control cluster
     */
    CHIP_ERROR LevelControl_Step(TargetEndpointInfo * endpoint, chip::app::Clusters::LevelControl::StepMode stepMode,
                                 uint8_t stepSize, uint16_t transitionTime, uint8_t optionMask, uint8_t optionOverride,
                                 std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR LevelControl_MoveToLevel(TargetEndpointInfo * endpoint, uint8_t level, uint16_t transitionTime, uint8_t optionMask,
                                        uint8_t optionOverride, std::function<void(CHIP_ERROR)> responseCallback);

    CHIP_ERROR
    LevelControl_SubscribeToCurrentLevel(
        TargetEndpointInfo * endpoint, void * context,
        chip::Controller::ReadResponseSuccessCallback<
            chip::app::Clusters::LevelControl::Attributes::CurrentLevel::TypeInfo::DecodableArgType>
            successFn,
        chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
        chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished);
    CHIP_ERROR
    LevelControl_SubscribeToMinLevel(TargetEndpointInfo * endpoint, void * context,
                                     chip::Controller::ReadResponseSuccessCallback<
                                         chip::app::Clusters::LevelControl::Attributes::MinLevel::TypeInfo::DecodableArgType>
                                         successFn,
                                     chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval,
                                     uint16_t maxInterval,
                                     chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished);
    CHIP_ERROR
    LevelControl_SubscribeToMaxLevel(TargetEndpointInfo * endpoint, void * context,
                                     chip::Controller::ReadResponseSuccessCallback<
                                         chip::app::Clusters::LevelControl::Attributes::MaxLevel::TypeInfo::DecodableArgType>
                                         successFn,
                                     chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval,
                                     uint16_t maxInterval,
                                     chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished);

    /**
     * @brief Media Playback cluster
     */
    CHIP_ERROR MediaPlayback_Play(TargetEndpointInfo * endpoint, std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR MediaPlayback_Pause(TargetEndpointInfo * endpoint, std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR MediaPlayback_StopPlayback(TargetEndpointInfo * endpoint, std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR MediaPlayback_Next(TargetEndpointInfo * endpoint, std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR MediaPlayback_Seek(TargetEndpointInfo * endpoint, uint64_t position,
                                  std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR MediaPlayback_SkipForward(TargetEndpointInfo * endpoint, uint64_t deltaPositionMilliseconds,
                                         std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR MediaPlayback_SkipBackward(TargetEndpointInfo * endpoint, uint64_t deltaPositionMilliseconds,
                                          std::function<void(CHIP_ERROR)> responseCallback);

    CHIP_ERROR MediaPlayback_SubscribeToCurrentState(
        TargetEndpointInfo * endpoint, void * context,
        chip::Controller::ReadResponseSuccessCallback<
            chip::app::Clusters::MediaPlayback::Attributes::CurrentState::TypeInfo::DecodableArgType>
            successFn,
        chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
        chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished);
    CHIP_ERROR
    MediaPlayback_SubscribeToStartTime(TargetEndpointInfo * endpoint, void * context,
                                       chip::Controller::ReadResponseSuccessCallback<
                                           chip::app::Clusters::MediaPlayback::Attributes::StartTime::TypeInfo::DecodableArgType>
                                           successFn,
                                       chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval,
                                       uint16_t maxInterval,
                                       chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished);
    CHIP_ERROR
    MediaPlayback_SubscribeToDuration(TargetEndpointInfo * endpoint, void * context,
                                      chip::Controller::ReadResponseSuccessCallback<
                                          chip::app::Clusters::MediaPlayback::Attributes::Duration::TypeInfo::DecodableArgType>
                                          successFn,
                                      chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval,
                                      uint16_t maxInterval,
                                      chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished);
    CHIP_ERROR MediaPlayback_SubscribeToSampledPosition(
        TargetEndpointInfo * endpoint, void * context,
        chip::Controller::ReadResponseSuccessCallback<
            chip::app::Clusters::MediaPlayback::Attributes::SampledPosition::TypeInfo::DecodableArgType>
            successFn,
        chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
        chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished);
    CHIP_ERROR MediaPlayback_SubscribeToPlaybackSpeed(
        TargetEndpointInfo * endpoint, void * context,
        chip::Controller::ReadResponseSuccessCallback<
            chip::app::Clusters::MediaPlayback::Attributes::PlaybackSpeed::TypeInfo::DecodableArgType>
            successFn,
        chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
        chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished);
    CHIP_ERROR MediaPlayback_SubscribeToSeekRangeEnd(
        TargetEndpointInfo * endpoint, void * context,
        chip::Controller::ReadResponseSuccessCallback<
            chip::app::Clusters::MediaPlayback::Attributes::SeekRangeEnd::TypeInfo::DecodableArgType>
            successFn,
        chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
        chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished);
    CHIP_ERROR MediaPlayback_SubscribeToSeekRangeStart(
        TargetEndpointInfo * endpoint, void * context,
        chip::Controller::ReadResponseSuccessCallback<
            chip::app::Clusters::MediaPlayback::Attributes::SeekRangeStart::TypeInfo::DecodableArgType>
            successFn,
        chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
        chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished);

    /**
     * @brief Application Launcher cluster
     */
    CHIP_ERROR ApplicationLauncher_LaunchApp(TargetEndpointInfo * endpoint,
                                             chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application,
                                             chip::Optional<chip::ByteSpan> data, std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR ApplicationLauncher_StopApp(TargetEndpointInfo * endpoint,
                                           chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application,
                                           std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR ApplicationLauncher_HideApp(TargetEndpointInfo * endpoint,
                                           chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application,
                                           std::function<void(CHIP_ERROR)> responseCallback);

    CHIP_ERROR
    ApplicationLauncher_SubscribeToCurrentApp(
        TargetEndpointInfo * endpoint, void * context,
        chip::Controller::ReadResponseSuccessCallback<
            chip::app::Clusters::ApplicationLauncher::Attributes::CurrentApp::TypeInfo::DecodableArgType>
            successFn,
        chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
        chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished);

    /**
     * @brief Target Navigator cluster
     */
    CHIP_ERROR TargetNavigator_NavigateTarget(TargetEndpointInfo * endpoint, const uint8_t target,
                                              const chip::Optional<chip::CharSpan> data,
                                              std::function<void(CHIP_ERROR)> responseCallback);

    CHIP_ERROR TargetNavigator_SubscribeToTargetList(
        TargetEndpointInfo * endpoint, void * context,
        chip::Controller::ReadResponseSuccessCallback<
            chip::app::Clusters::TargetNavigator::Attributes::TargetList::TypeInfo::DecodableArgType>
            successFn,
        chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
        chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished);
    CHIP_ERROR TargetNavigator_SubscribeToCurrentTarget(
        TargetEndpointInfo * endpoint, void * context,
        chip::Controller::ReadResponseSuccessCallback<
            chip::app::Clusters::TargetNavigator::Attributes::CurrentTarget::TypeInfo::DecodableArgType>
            successFn,
        chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
        chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished);

    /**
     * @brief Keypad Input cluster
     */
    CHIP_ERROR KeypadInput_SendKey(TargetEndpointInfo * endpoint, const chip::app::Clusters::KeypadInput::CecKeyCode keyCode,
                                   std::function<void(CHIP_ERROR)> responseCallback);

    /**
     * @brief Application Basic cluster
     */
    CHIP_ERROR ApplicationBasic_SubscribeToVendorName(
        TargetEndpointInfo * endpoint, void * context,
        chip::Controller::ReadResponseSuccessCallback<
            chip::app::Clusters::ApplicationBasic::Attributes::VendorName::TypeInfo::DecodableArgType>
            successFn,
        chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
        chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished);
    CHIP_ERROR ApplicationBasic_SubscribeToVendorID(
        TargetEndpointInfo * endpoint, void * context,
        chip::Controller::ReadResponseSuccessCallback<
            chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo::DecodableArgType>
            successFn,
        chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
        chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished);
    CHIP_ERROR ApplicationBasic_SubscribeToApplicationName(
        TargetEndpointInfo * endpoint, void * context,
        chip::Controller::ReadResponseSuccessCallback<
            chip::app::Clusters::ApplicationBasic::Attributes::ApplicationName::TypeInfo::DecodableArgType>
            successFn,
        chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
        chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished);
    CHIP_ERROR ApplicationBasic_SubscribeToProductID(
        TargetEndpointInfo * endpoint, void * context,
        chip::Controller::ReadResponseSuccessCallback<
            chip::app::Clusters::ApplicationBasic::Attributes::ProductID::TypeInfo::DecodableArgType>
            successFn,
        chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
        chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished);
    CHIP_ERROR ApplicationBasic_SubscribeToApplication(
        TargetEndpointInfo * endpoint, void * context,
        chip::Controller::ReadResponseSuccessCallback<
            chip::app::Clusters::ApplicationBasic::Attributes::Application::TypeInfo::DecodableArgType>
            successFn,
        chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
        chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished);
    CHIP_ERROR
    ApplicationBasic_SubscribeToStatus(TargetEndpointInfo * endpoint, void * context,
                                       chip::Controller::ReadResponseSuccessCallback<
                                           chip::app::Clusters::ApplicationBasic::Attributes::Status::TypeInfo::DecodableArgType>
                                           successFn,
                                       chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval,
                                       uint16_t maxInterval,
                                       chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished);
    CHIP_ERROR ApplicationBasic_SubscribeToApplicationVersion(
        TargetEndpointInfo * endpoint, void * context,
        chip::Controller::ReadResponseSuccessCallback<
            chip::app::Clusters::ApplicationBasic::Attributes::ApplicationVersion::TypeInfo::DecodableArgType>
            successFn,
        chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
        chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished);
    CHIP_ERROR ApplicationBasic_SubscribeToAllowedVendorList(
        TargetEndpointInfo * endpoint, void * context,
        chip::Controller::ReadResponseSuccessCallback<
            chip::app::Clusters::ApplicationBasic::Attributes::AllowedVendorList::TypeInfo::DecodableArgType>
            successFn,
        chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
        chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished);

    CHIP_ERROR
    ApplicationBasic_ReadVendorName(TargetEndpointInfo * endpoint, void * context,
                                    chip::Controller::ReadResponseSuccessCallback<
                                        chip::app::Clusters::ApplicationBasic::Attributes::VendorName::TypeInfo::DecodableArgType>
                                        successFn,
                                    chip::Controller::ReadResponseFailureCallback failureFn);
    CHIP_ERROR
    ApplicationBasic_ReadVendorID(TargetEndpointInfo * endpoint, void * context,
                                  chip::Controller::ReadResponseSuccessCallback<
                                      chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo::DecodableArgType>
                                      successFn,
                                  chip::Controller::ReadResponseFailureCallback failureFn);
    CHIP_ERROR ApplicationBasic_ReadApplicationName(
        TargetEndpointInfo * endpoint, void * context,
        chip::Controller::ReadResponseSuccessCallback<
            chip::app::Clusters::ApplicationBasic::Attributes::ApplicationName::TypeInfo::DecodableArgType>
            successFn,
        chip::Controller::ReadResponseFailureCallback failureFn);
    CHIP_ERROR
    ApplicationBasic_ReadProductID(TargetEndpointInfo * endpoint, void * context,
                                   chip::Controller::ReadResponseSuccessCallback<
                                       chip::app::Clusters::ApplicationBasic::Attributes::ProductID::TypeInfo::DecodableArgType>
                                       successFn,
                                   chip::Controller::ReadResponseFailureCallback failureFn);
    CHIP_ERROR
    ApplicationBasic_ReadApplication(TargetEndpointInfo * endpoint, void * context,
                                     chip::Controller::ReadResponseSuccessCallback<
                                         chip::app::Clusters::ApplicationBasic::Attributes::Application::TypeInfo::DecodableArgType>
                                         successFn,
                                     chip::Controller::ReadResponseFailureCallback failureFn);
    CHIP_ERROR
    ApplicationBasic_ReadStatus(TargetEndpointInfo * endpoint, void * context,
                                chip::Controller::ReadResponseSuccessCallback<
                                    chip::app::Clusters::ApplicationBasic::Attributes::Status::TypeInfo::DecodableArgType>
                                    successFn,
                                chip::Controller::ReadResponseFailureCallback failureFn);
    CHIP_ERROR ApplicationBasic_ReadApplicationVersion(
        TargetEndpointInfo * endpoint, void * context,
        chip::Controller::ReadResponseSuccessCallback<
            chip::app::Clusters::ApplicationBasic::Attributes::ApplicationVersion::TypeInfo::DecodableArgType>
            successFn,
        chip::Controller::ReadResponseFailureCallback failureFn);
    CHIP_ERROR ApplicationBasic_ReadAllowedVendorList(
        TargetEndpointInfo * endpoint, void * context,
        chip::Controller::ReadResponseSuccessCallback<
            chip::app::Clusters::ApplicationBasic::Attributes::AllowedVendorList::TypeInfo::DecodableArgType>
            successFn,
        chip::Controller::ReadResponseFailureCallback failureFn);

    /*
     * @brief Channel cluster
     */
    CHIP_ERROR Channel_ChangeChannelCommand(TargetEndpointInfo * endpoint, const chip::CharSpan & match,
                                            std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR Channel_SubscribeToLineup(
        TargetEndpointInfo * endpoint, void * context,
        chip::Controller::ReadResponseSuccessCallback<chip::app::Clusters::Channel::Attributes::Lineup::TypeInfo::DecodableArgType>
            successFn,
        chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
        chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished);

private:
    static CastingServer * castingServer_;
    CastingServer();

    CHIP_ERROR InitBindingHandlers();
    static void DeviceEventCallback(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
    void ReadServerClusters(chip::EndpointId endpointId);

    PersistenceManager mPersistenceManager;
    bool mInited = false;
    TargetVideoPlayerInfo mActiveTargetVideoPlayerInfo;
    TargetVideoPlayerInfo mCachedTargetVideoPlayerInfo[kMaxCachedVideoPlayers];
    uint16_t mTargetVideoPlayerVendorId                                   = 0;
    uint16_t mTargetVideoPlayerProductId                                  = 0;
    uint16_t mTargetVideoPlayerDeviceType                                 = 0;
    char mTargetVideoPlayerDeviceName[chip::Dnssd::kMaxDeviceNameLen + 1] = {};
    size_t mTargetVideoPlayerNumIPs                                       = 0; // number of valid IP addresses
    chip::Inet::IPAddress mTargetVideoPlayerIpAddress[chip::Dnssd::CommonResolutionData::kMaxIPAddresses];

    chip::Controller::CommissionableNodeController mCommissionableNodeController;
    std::function<void(CHIP_ERROR)> mCommissioningCompleteCallback;

    std::function<void(TargetEndpointInfo *)> mOnNewOrUpdatedEndpoint;
    std::function<void(TargetVideoPlayerInfo *)> mOnConnectionSuccessClientCallback;
    std::function<void(CHIP_ERROR)> mOnConnectionFailureClientCallback;

    /**
     * @brief Content Launcher cluster
     */
    LaunchURLCommand mLaunchURLCommand;
    LaunchContentCommand mLaunchContentCommand;

    AcceptHeaderSubscriber mAcceptHeaderSubscriber;
    SupportedStreamingProtocolsSubscriber mSupportedStreamingProtocolsSubscriber;

    /**
     * @brief Level Control cluster
     */
    StepCommand mStepCommand;
    MoveToLevelCommand mMoveToLevelCommand;

    CurrentLevelSubscriber mCurrentLevelSubscriber;
    MinLevelSubscriber mMinLevelSubscriber;
    MaxLevelSubscriber mMaxLevelSubscriber;

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

    CurrentStateSubscriber mCurrentStateSubscriber;
    StartTimeSubscriber mStartTimeSubscriber;
    DurationSubscriber mDurationSubscriber;
    SampledPositionSubscriber mSampledPositionSubscriber;
    PlaybackSpeedSubscriber mPlaybackSpeedSubscriber;
    SeekRangeEndSubscriber mSeekRangeEndSubscriber;
    SeekRangeStartSubscriber mSeekRangeStartSubscriber;

    /**
     * @brief Application Launcher cluster
     */
    LaunchAppCommand mLaunchAppCommand;
    StopAppCommand mStopAppCommand;
    HideAppCommand mHideAppCommand;

    CurrentAppSubscriber mCurrentAppSubscriber;

    /**
     * @brief Target Navigator cluster
     */
    NavigateTargetCommand mNavigateTargetCommand;

    TargetListSubscriber mTargetListSubscriber;
    CurrentTargetSubscriber mCurrentTargetSubscriber;

    /**
     * @brief Keypad Input cluster
     */
    SendKeyCommand mSendKeyCommand;

    /**
     * @brief Application Basic cluster
     */
    VendorNameSubscriber mVendorNameSubscriber;
    VendorIDSubscriber mVendorIDSubscriber;
    ApplicationNameSubscriber mApplicationNameSubscriber;
    ProductIDSubscriber mProductIDSubscriber;
    ApplicationSubscriber mApplicationSubscriber;
    StatusSubscriber mStatusSubscriber;
    ApplicationVersionSubscriber mApplicationVersionSubscriber;
    AllowedVendorListSubscriber mAllowedVendorListSubscriber;

    VendorNameReader mVendorNameReader;
    VendorIDReader mVendorIDReader;
    ApplicationNameReader mApplicationNameReader;
    ProductIDReader mProductIDReader;
    ApplicationReader mApplicationReader;
    StatusReader mStatusReader;
    ApplicationVersionReader mApplicationVersionReader;
    AllowedVendorListReader mAllowedVendorListReader;

    /*
     * @brief Channel cluster
     */
    ChangeChannelCommand mChangeChannelCommand;

    LineupSubscriber mLineupSubscriber;
};
