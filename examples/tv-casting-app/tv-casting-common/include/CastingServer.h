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
#include "CommissioningCallbacks.h"
#include "ContentLauncher.h"
#include "KeypadInput.h"
#include "LevelControl.h"
#include "MediaPlayback.h"
#include "Messages.h"
#include "OnOff.h"
#include "PersistenceManager.h"
#include "TargetEndpointInfo.h"
#include "TargetNavigator.h"
#include "TargetVideoPlayerInfo.h"
#include "WakeOnLan.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/server/AppDelegate.h>
#include <app/server/Server.h>
#include <controller/CHIPCluster.h>
#include <controller/CHIPCommissionableNodeController.h>
#include <functional>

inline constexpr chip::System::Clock::Seconds16 kCommissioningWindowTimeout = chip::System::Clock::Seconds16(3 * 60);

/**
 * @brief Represents a TV Casting server that can get the casting app commissioned
 *  and then have it send TV Casting/Media related commands. This is to be instantiated
 *  as a singleton and is to be used across Linux, Android and iOS.
 */
class CastingServer : public AppDelegate
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    ,
                      chip::Protocols::UserDirectedCommissioning::CommissionerDeclarationHandler
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
{
public:
    CastingServer(CastingServer & other)  = delete;
    void operator=(const CastingServer &) = delete;
    static CastingServer * GetInstance();

    CHIP_ERROR PreInit(AppParams * AppParams = nullptr);
    CHIP_ERROR Init(AppParams * AppParams = nullptr);
    CHIP_ERROR InitBindingHandlers();
    void InitAppDelegation();

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    void SetCommissionerPasscodeEnabled(bool enabled) { mUdcCommissionerPasscodeEnabled = enabled; };
    void SetCommissionerPasscodeReady() { mUdcCommissionerPasscodeReady = true; };
    void OnCommissionerDeclarationMessage(const chip::Transport::PeerAddress & source,
                                          chip::Protocols::UserDirectedCommissioning::CommissionerDeclaration cd) override;
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

    CHIP_ERROR DiscoverCommissioners(chip::Controller::DeviceDiscoveryDelegate * deviceDiscoveryDelegate = nullptr);
    const chip::Dnssd::CommissionNodeData *
    GetDiscoveredCommissioner(int index, chip::Optional<TargetVideoPlayerInfo *> & outAssociatedConnectableVideoPlayer);
    CHIP_ERROR OpenBasicCommissioningWindow(CommissioningCallbacks commissioningCallbacks,
                                            std::function<void(TargetVideoPlayerInfo *)> onConnectionSuccess,
                                            std::function<void(CHIP_ERROR)> onConnectionFailure,
                                            std::function<void(TargetEndpointInfo *)> onNewOrUpdatedEndpoint);

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    CHIP_ERROR SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress commissioner);
    CHIP_ERROR SendUserDirectedCommissioningRequest(chip::Dnssd::CommissionNodeData * selectedCommissioner);
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

    void LogCachedVideoPlayers();
    CHIP_ERROR AddVideoPlayer(TargetVideoPlayerInfo * targetVideoPlayerInfo);

    CHIP_ERROR SendWakeOnLan(TargetVideoPlayerInfo & targetVideoPlayerInfo);

    CHIP_ERROR PurgeCache();

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
        chip::Optional<chip::app::Clusters::ContentLauncher::Structs::BrandingInformationStruct::Type> brandingInformation,
        std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR ContentLauncher_LaunchContent(TargetEndpointInfo * endpoint,
                                             chip::app::Clusters::ContentLauncher::Structs::ContentSearchStruct::Type search,
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
    CHIP_ERROR LevelControl_Step(TargetEndpointInfo * endpoint, chip::app::Clusters::LevelControl::StepModeEnum stepMode,
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
     * @brief OnOff cluster
     */
    CHIP_ERROR OnOff_On(TargetEndpointInfo * endpoint, std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR OnOff_Off(TargetEndpointInfo * endpoint, std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR OnOff_Toggle(TargetEndpointInfo * endpoint, std::function<void(CHIP_ERROR)> responseCallback);

    /**
     * @brief Messages cluster
     */
    CHIP_ERROR Messages_PresentMessagesRequest(TargetEndpointInfo * endpoint, const char * messageText,
                                               std::function<void(CHIP_ERROR)> responseCallback);

    /**
     * @brief Media Playback cluster
     */
    CHIP_ERROR MediaPlayback_Play(TargetEndpointInfo * endpoint, std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR MediaPlayback_Pause(TargetEndpointInfo * endpoint, std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR MediaPlayback_StopPlayback(TargetEndpointInfo * endpoint, std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR MediaPlayback_Next(TargetEndpointInfo * endpoint, std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR MediaPlayback_Previous(TargetEndpointInfo * endpoint, std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR MediaPlayback_Rewind(TargetEndpointInfo * endpoint, std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR MediaPlayback_FastForward(TargetEndpointInfo * endpoint, std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR MediaPlayback_StartOver(TargetEndpointInfo * endpoint, std::function<void(CHIP_ERROR)> responseCallback);
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
                                             chip::app::Clusters::ApplicationLauncher::Structs::ApplicationStruct::Type application,
                                             chip::Optional<chip::ByteSpan> data, std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR ApplicationLauncher_StopApp(TargetEndpointInfo * endpoint,
                                           chip::app::Clusters::ApplicationLauncher::Structs::ApplicationStruct::Type application,
                                           std::function<void(CHIP_ERROR)> responseCallback);
    CHIP_ERROR ApplicationLauncher_HideApp(TargetEndpointInfo * endpoint,
                                           chip::app::Clusters::ApplicationLauncher::Structs::ApplicationStruct::Type application,
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
    CHIP_ERROR KeypadInput_SendKey(TargetEndpointInfo * endpoint, const chip::app::Clusters::KeypadInput::CECKeyCodeEnum keyCode,
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

    CHIP_ERROR SetRotatingDeviceIdUniqueId(chip::Optional<chip::ByteSpan> rotatingDeviceIdUniqueId);

    static void DeviceEventCallback(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
    void ReadServerClusters(chip::EndpointId endpointId);

    void OnCommissioningSessionEstablishmentStarted() override;
    void OnCommissioningSessionStarted() override;
    void OnCommissioningSessionEstablishmentError(CHIP_ERROR err) override;
    void OnCommissioningSessionStopped() override;
    void OnCommissioningWindowOpened() override {}
    void OnCommissioningWindowClosed() override {}

    static void VerifyOrEstablishConnectionTask(chip::System::Layer * aSystemLayer, void * context);
    CHIP_ERROR ReadMACAddress(TargetEndpointInfo * endpoint);

    /**
     * @brief Retrieve the IP Address to use for the UDC request.
     * This function will look for an IPv4 address in the list of IPAddresses passed in if available and return
     * that address if found. If there are no available IPv4 addresses, it will default to the first available address.
     * This logic is similar to the one used by the UDC server that prefers IPv4 addresses.
     *
     * @param ipAddresses - The list of ip addresses available to use
     * @param numIPs - The number of ip addresses available in the array
     *
     * @returns The IPv4 address in the array if available, otherwise will return the first address in the list.
     */
    static chip::Inet::IPAddress * getIpAddressForUDCRequest(chip::Inet::IPAddress ipAddresses[], const size_t numIPs);

    PersistenceManager mPersistenceManager;
    bool mInited        = false;
    bool mUdcInProgress = false;
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    bool mUdcCommissionerPasscodeEnabled                                                           = false;
    bool mUdcCommissionerPasscodeReady                                                             = false;
    char mUdcCommissionerPasscodeInstanceName[chip::Dnssd::Commission::kInstanceNameMaxLength + 1] = "";
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

    chip::Dnssd::DiscoveredNodeData mStrNodeDataList[kMaxCachedVideoPlayers];
    TargetVideoPlayerInfo mActiveTargetVideoPlayerInfo;
    TargetVideoPlayerInfo mCachedTargetVideoPlayerInfo[kMaxCachedVideoPlayers];
    uint16_t mTargetVideoPlayerVendorId                                                      = 0;
    uint16_t mTargetVideoPlayerProductId                                                     = 0;
    uint16_t mTargetVideoPlayerPort                                                          = 0;
    chip::DeviceTypeId mTargetVideoPlayerDeviceType                                          = 0;
    char mTargetVideoPlayerDeviceName[chip::Dnssd::kMaxDeviceNameLen + 1]                    = {};
    char mTargetVideoPlayerHostName[chip::Dnssd::kHostNameMaxLength + 1]                     = {};
    char mTargetVideoPlayerInstanceName[chip::Dnssd::Commission::kInstanceNameMaxLength + 1] = {};
    size_t mTargetVideoPlayerNumIPs                                                          = 0; // number of valid IP addresses
    chip::Inet::IPAddress mTargetVideoPlayerIpAddress[chip::Dnssd::CommonResolutionData::kMaxIPAddresses];

    chip::Controller::CommissionableNodeController mCommissionableNodeController;

    CommissioningCallbacks mCommissioningCallbacks;

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
     * @brief OnOff cluster
     */
    OnCommand mOnCommand;
    OffCommand mOffCommand;
    ToggleCommand mToggleCommand;

    /**
     * @brief OnOff cluster
     */
    PresentMessagesRequestCommand mPresentMessagesRequestCommand;

    /**
     * @brief Media Playback cluster
     */
    PlayCommand mPlayCommand;
    PauseCommand mPauseCommand;
    StopPlaybackCommand mStopPlaybackCommand;
    NextCommand mNextCommand;
    PreviousCommand mPreviousCommand;
    RewindCommand mRewindCommand;
    FastForwardCommand mFastForwardCommand;
    StartOverCommand mStartOverCommand;
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

    /**
     * @brief WakeOnLan cluster
     */
    MACAddressReader mMACAddressReader;
};
