/*
 *
 *    Copyright (c) 2022-2026 Project CHIP Authors
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

#include "CastingUtils.h"

#include "CommissioningCallbacks.h"

#include <LinuxCommissionableDataProvider.h>
#include <memory>
#include <setup_payload/SetupPayload.h>
#include <string>
#include <unordered_map>
#include <vector>

using namespace chip;
using namespace chip::System;
using namespace chip::DeviceLayer;
using namespace chip::Dnssd;

// TODO: Accept these values over CLI
const char kContentUrl[]        = "https://www.test.com/videoid";
const char kContentDisplayStr[] = "Test video";
int gInitialContextVal          = 121212;

namespace {

std::unique_ptr<LinuxCommissionableDataProvider> gFixedPasscodeCommissionableDataProvider;
CommissionableDataProvider * gOriginalCommissionableDataProvider = nullptr;
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
uint32_t gFixedPasscodeCommissioningPasscode = 1234;
bool gFixedPasscodeCommissioningSelected     = false;
bool gFixedPasscodeCommissioningEnabled      = false;
std::string gFixedPasscodeCommissioningCastingPlayerId;
std::vector<Inet::IPAddress> gFixedPasscodeCommissioningCastingPlayerAddresses;
std::unordered_map<std::string, uint32_t> gRejectedFixedPasscodes;
uint32_t gFixedPasscodeCommissioningAttemptPasscode = 0;

bool CommissionerDeclarationMatchesActiveCastingPlayer(const chip::Transport::PeerAddress & source)
{
    for (const auto & address : gFixedPasscodeCommissioningCastingPlayerAddresses)
    {
        if (address == source.GetIPAddress())
        {
            return true;
        }
    }
    return false;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

void RestoreCommissionableDataProvider()
{
    if (gOriginalCommissionableDataProvider != nullptr)
    {
        DeviceLayer::SetCommissionableDataProvider(gOriginalCommissionableDataProvider);
        gOriginalCommissionableDataProvider = nullptr;
        gFixedPasscodeCommissionableDataProvider.reset();
    }
}

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
void HandleCommissionerDeclaration(const chip::Transport::PeerAddress & source,
                                   chip::Protocols::UserDirectedCommissioning::CommissionerDeclaration commissionerDeclaration)
{
    bool sourceMatchesActiveCastingPlayer = CommissionerDeclarationMatchesActiveCastingPlayer(source);
    if (commissionerDeclaration.GetErrorCode() ==
            chip::Protocols::UserDirectedCommissioning::CommissionerDeclaration::CdError::kPaseAuthFailed &&
        gFixedPasscodeCommissioningSelected)
    {
        if (!sourceMatchesActiveCastingPlayer)
        {
            ChipLogError(AppServer, "Ignoring PASE authentication failure from a non-target commissioner");
            return;
        }
        gRejectedFixedPasscodes[gFixedPasscodeCommissioningCastingPlayerId] = gFixedPasscodeCommissioningAttemptPasscode;
        gFixedPasscodeCommissioningAttemptPasscode                          = 0;
        gFixedPasscodeCommissioningSelected                                 = false;
        RestoreCommissionableDataProvider();
        ChipLogError(AppServer,
                     "The stored fixed setup passcode does not match this commissioner. Update the stored passcode or use another "
                     "passcode flow.");
        return;
    }

    if (commissionerDeclaration.GetCancelPasscode())
    {
        if (!sourceMatchesActiveCastingPlayer)
        {
            ChipLogError(AppServer, "Ignoring cancellation from a non-target commissioner");
            return;
        }
        chip::Server::GetInstance().GetCommissioningWindowManager().CloseCommissioningWindow();
        gFixedPasscodeCommissioningAttemptPasscode = 0;
        gFixedPasscodeCommissioningSelected        = false;
        RestoreCommissionableDataProvider();
        ChipLogError(AppServer, "The commissioner canceled the commissioning attempt");
        return;
    }

    if (!commissionerDeclaration.GetFixedPasscodeCommissioningSelected())
    {
        return;
    }

    gFixedPasscodeCommissioningSelected          = true;
    CommissionableDataProvider * currentProvider = DeviceLayer::GetCommissionableDataProvider();
    if (currentProvider == nullptr)
    {
        ChipLogError(AppServer, "Fixed Passcode Commissioning has no CommissionableDataProvider");
        gFixedPasscodeCommissioningSelected = false;
        TEMPORARY_RETURN_IGNORED CastingServer::GetInstance()->CancelFixedPasscodeCommissioning(source);
        return;
    }

    gFixedPasscodeCommissionableDataProvider = std::make_unique<LinuxCommissionableDataProvider>();
    uint16_t discriminator                   = 0;
    CHIP_ERROR err                           = currentProvider->GetSetupDiscriminator(discriminator);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to read setup discriminator: %" CHIP_ERROR_FORMAT, err.Format());
        gFixedPasscodeCommissioningSelected = false;
        gFixedPasscodeCommissionableDataProvider.reset();
        TEMPORARY_RETURN_IGNORED CastingServer::GetInstance()->CancelFixedPasscodeCommissioning(source);
        return;
    }

    chip::Optional<std::vector<uint8_t>> noVerifier;
    chip::Optional<std::vector<uint8_t>> noSalt;
    err = gFixedPasscodeCommissionableDataProvider->Init(noVerifier, noSalt, chip::Crypto::kSpake2p_Min_PBKDF_Iterations,
                                                         chip::MakeOptional(gFixedPasscodeCommissioningPasscode), discriminator);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to configure fixed passcode: %" CHIP_ERROR_FORMAT, err.Format());
        gFixedPasscodeCommissioningSelected = false;
        gFixedPasscodeCommissionableDataProvider.reset();
        TEMPORARY_RETURN_IGNORED CastingServer::GetInstance()->CancelFixedPasscodeCommissioning(source);
        return;
    }

    gFixedPasscodeCommissioningAttemptPasscode = gFixedPasscodeCommissioningPasscode;
    gOriginalCommissionableDataProvider        = currentProvider;
    err = CastingServer::GetInstance()->ContinueFixedPasscodeCommissioning(source, gFixedPasscodeCommissionableDataProvider.get());
    if (err != CHIP_NO_ERROR)
    {
        gFixedPasscodeCommissioningAttemptPasscode = 0;
        gFixedPasscodeCommissioningSelected        = false;
        RestoreCommissionableDataProvider();
        TEMPORARY_RETURN_IGNORED CastingServer::GetInstance()->CancelFixedPasscodeCommissioning(source);
        ChipLogError(AppServer, "Failed to continue Fixed Passcode Commissioning: %" CHIP_ERROR_FORMAT, err.Format());
    }
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

} // namespace

CHIP_ERROR DiscoverCommissioners()
{
    // Send discover commissioners request
    ReturnErrorOnFailure(CastingServer::GetInstance()->DiscoverCommissioners());

    // Give commissioners some time to respond and then ScheduleWork to initiate commissioning
    return DeviceLayer::SystemLayer().StartTimer(
        chip::System::Clock::Milliseconds32(kCommissionerDiscoveryTimeoutInMs),
        [](System::Layer *, void *) {
            TEMPORARY_RETURN_IGNORED chip::DeviceLayer::PlatformMgr().ScheduleWork(InitCommissioningFlow);
        },
        nullptr);
}

CHIP_ERROR RequestCommissioning(int index)
{
    chip::Optional<TargetVideoPlayerInfo *> associatedConnectableVideoPlayer;
    const Dnssd::CommissionNodeData * selectedCommissioner =
        CastingServer::GetInstance()->GetDiscoveredCommissioner(index, associatedConnectableVideoPlayer);
    if (selectedCommissioner == nullptr)
    {
        ChipLogError(AppServer, "No such commissioner with index %d exists", index);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    std::string castingPlayerId = selectedCommissioner->instanceName;
    auto rejectedPasscode       = gRejectedFixedPasscodes.find(castingPlayerId);
    bool fixedPasscodeRejected =
        rejectedPasscode != gRejectedFixedPasscodes.end() && rejectedPasscode->second == gFixedPasscodeCommissioningPasscode;
    CastingServer::GetInstance()->SetFixedPasscodeCommissioningEnabled(gFixedPasscodeCommissioningEnabled &&
                                                                       !fixedPasscodeRejected);
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    PrepareForCommissioning(selectedCommissioner);
    return CHIP_NO_ERROR;
}

CHIP_ERROR SetUseFixedPasscodeCommissioning(bool enabled)
{
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    gFixedPasscodeCommissioningEnabled = enabled;
    CastingServer::GetInstance()->SetFixedPasscodeCommissioningEnabled(enabled);
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    ChipLogProgress(AppServer, "Fixed Passcode Commissioning %s", enabled ? "enabled" : "disabled");
    return CHIP_NO_ERROR;
}

CHIP_ERROR SetFixedPasscodeCommissioningPasscode(uint32_t passcode)
{
    VerifyOrReturnError(gOriginalCommissionableDataProvider == nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(SetupPayload::IsValidSetupPIN(passcode), CHIP_ERROR_INVALID_ARGUMENT);

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    gFixedPasscodeCommissioningPasscode = passcode;
    gRejectedFixedPasscodes.clear();
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    ChipLogProgress(AppServer, "Updated the remembered Fixed Passcode Commissioning passcode");
    return CHIP_NO_ERROR;
}

/**
 * Enters commissioning mode, opens commissioning window, logs onboarding payload.
 * If non-null selectedCommissioner is provided, sends user directed commissioning
 * request to the selectedCommissioner and advertises self as commissionable node over DNS-SD
 */
void PrepareForCommissioning(const Dnssd::CommissionNodeData * selectedCommissioner)
{
    TEMPORARY_RETURN_IGNORED CastingServer::GetInstance()->Init();

    RestoreCommissionableDataProvider();
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    gFixedPasscodeCommissioningSelected        = false;
    gFixedPasscodeCommissioningAttemptPasscode = 0;
    gFixedPasscodeCommissioningCastingPlayerId.clear();
    gFixedPasscodeCommissioningCastingPlayerAddresses.clear();
    if (selectedCommissioner != nullptr)
    {
        gFixedPasscodeCommissioningCastingPlayerId = selectedCommissioner->instanceName;
        gFixedPasscodeCommissioningCastingPlayerAddresses.assign(selectedCommissioner->ipAddress,
                                                                 selectedCommissioner->ipAddress + selectedCommissioner->numIPs);
    }
    CastingServer::GetInstance()->SetCommissionerDeclarationCallback(HandleCommissionerDeclaration);
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

    CommissioningCallbacks commissioningCallbacks;
    commissioningCallbacks.sessionEstablished = []() {
        RestoreCommissionableDataProvider();
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
        gFixedPasscodeCommissioningSelected = false;
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    };
    commissioningCallbacks.sessionEstablishmentError   = [](CHIP_ERROR) { RestoreCommissionableDataProvider(); };
    commissioningCallbacks.sessionEstablishmentStopped = RestoreCommissionableDataProvider;
    commissioningCallbacks.commissioningWindowClosed   = RestoreCommissionableDataProvider;
    commissioningCallbacks.commissioningComplete       = HandleCommissioningCompleteCallback;
    TEMPORARY_RETURN_IGNORED CastingServer::GetInstance()->OpenBasicCommissioningWindow(
        commissioningCallbacks, OnConnectionSuccess, OnConnectionFailure, OnNewOrUpdatedEndpoint);

    // Display onboarding payload
    chip::DeviceLayer::ConfigurationMgr().LogDeviceConfig();

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    if (selectedCommissioner != nullptr)
    {
        // Send User Directed commissioning request
        // Wait 1 second to allow our commissionee DNS records to publish (needed on Mac)
        int32_t expiration = 1;
        ReturnOnFailure(DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(expiration), HandleUDCSendExpiration,
                                                              (void *) selectedCommissioner));
    }
    else
    {
        ChipLogProgress(AppServer, "To run discovery again, enter: cast discover");
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
}

void InitCommissioningFlow(intptr_t commandArg)
{
    int commissionerCount = 0;

    // Display discovered commissioner TVs to ask user to select one
    for (int i = 0; i < CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES; i++)
    {
        chip::Optional<TargetVideoPlayerInfo *> associatedConnectableVideoPlayer;
        const Dnssd::CommissionNodeData * commissioner =
            CastingServer::GetInstance()->GetDiscoveredCommissioner(i, associatedConnectableVideoPlayer);
        if (commissioner != nullptr)
        {
            ChipLogProgress(AppServer, "Discovered Commissioner #%d", commissionerCount);
            commissionerCount++;
            commissioner->LogDetail();
            if (associatedConnectableVideoPlayer.HasValue())
            {
                [[maybe_unused]] TargetVideoPlayerInfo * targetVideoPlayerInfo = associatedConnectableVideoPlayer.Value();
                ChipLogProgress(AppServer, "Previously connected with nodeId 0x" ChipLogFormatX64 " fabricIndex: %d",
                                ChipLogValueX64(targetVideoPlayerInfo->GetNodeId()), targetVideoPlayerInfo->GetFabricIndex());
            }
        }
    }

    if (commissionerCount > 0)
    {
        ChipLogProgress(AppServer, "%d commissioner(s) discovered. Select one (by number# above) to request commissioning from: ",
                        commissionerCount);

        ChipLogProgress(AppServer, "Example: cast request 0");
    }
    else
    {
        ChipLogError(AppServer, "No commissioner discovered, commissioning must be initiated manually!");
        PrepareForCommissioning();
    }
}

void LaunchURLResponseCallback(CHIP_ERROR err)
{
    ChipLogProgress(AppServer, "LaunchURLResponseCallback called with %" CHIP_ERROR_FORMAT, err.Format());
}

void OnCurrentStateReadResponseSuccess(
    void * context, chip::app::Clusters::MediaPlayback::Attributes::CurrentState::TypeInfo::DecodableArgType responseData)
{
    ChipLogProgress(AppServer, "OnCurrentStateReadResponseSuccess called with responseData: %d", static_cast<int>(responseData));
    switch (responseData)
    {
    case chip::app::Clusters::MediaPlayback::PlaybackStateEnum::kPlaying:
        ChipLogProgress(AppServer, "OnCurrentStateReadResponseSuccess CurrentState: Playing");
        break;
    case chip::app::Clusters::MediaPlayback::PlaybackStateEnum::kPaused:
        ChipLogProgress(AppServer, "OnCurrentStateReadResponseSuccess CurrentState: Paused");
        break;
    case chip::app::Clusters::MediaPlayback::PlaybackStateEnum::kNotPlaying:
        ChipLogProgress(AppServer, "OnCurrentStateReadResponseSuccess CurrentState: Not Playing");
        break;
    case chip::app::Clusters::MediaPlayback::PlaybackStateEnum::kBuffering:
        ChipLogProgress(AppServer, "OnCurrentStateReadResponseSuccess CurrentState: Buffering");
        break;
    default:
        ChipLogError(AppServer, "OnCurrentStateReadResponseSuccess Invalid CurrentState!");
        break;
    }

    if (context != nullptr)
    {
        ChipLogProgress(AppServer, "OnCurrentStateReadResponseSuccess context value: %d", *(static_cast<int *>(context)));
    }
}

void OnCurrentStateReadResponseFailure(void * context, CHIP_ERROR err)
{
    ChipLogProgress(AppServer, "OnCurrentStateReadResponseFailure called with %" CHIP_ERROR_FORMAT, err.Format());
}

void OnCurrentStateSubscriptionEstablished(void * context, SubscriptionId aSubscriptionId)
{
    ChipLogProgress(AppServer, "OnCurrentStateSubscriptionEstablished called");
    if (context != nullptr)
    {
        ChipLogProgress(AppServer, "OnCurrentStateSubscriptionEstablished context value: %d", *(static_cast<int *>(context)));
    }
}

void doCastingDemoActions(TargetEndpointInfo * endpoint)
{
    if (endpoint != nullptr && endpoint->IsInitialized())
    {
        if (endpoint->HasCluster(chip::app::Clusters::MediaPlayback::Id))
        {
            // Subscribe to MediaPlayback::CurrentState
            ChipLogProgress(AppServer,
                            "doCastingDemoActions requesting subscription on MediaPlayback:CurrentState on endpoint ID: %d",
                            endpoint->GetEndpointId());
            CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SubscribeToCurrentState(
                endpoint, static_cast<void *>(&gInitialContextVal), OnCurrentStateReadResponseSuccess,
                OnCurrentStateReadResponseFailure, 0, 4000, OnCurrentStateSubscriptionEstablished);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "MediaPlayback_SubscribeToCurrentState call failed!");
            }
        }
        else
        {
            ChipLogProgress(AppServer,
                            "doCastingDemoActions: Not subscribing to MediaPlayback:CurrentState on endpoint ID %d as it does not "
                            "support the MediaPlayback cluster",
                            endpoint->GetEndpointId());
        }

        if (endpoint->HasCluster(chip::app::Clusters::ContentLauncher::Id))
        {
            // Send a ContentLauncher::LaunchURL command
            ChipLogProgress(AppServer, "doCastingDemoActions sending ContentLauncher:LaunchURL on endpoint ID: %d",
                            endpoint->GetEndpointId());
            CHIP_ERROR err = CastingServer::GetInstance()->ContentLauncherLaunchURL(endpoint, kContentUrl, kContentDisplayStr,
                                                                                    LaunchURLResponseCallback);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "ContentLauncherLaunchURL call failed!");
            }
        }
        else
        {
            ChipLogProgress(AppServer,
                            "doCastingDemoActions: Not sending ContentLauncher:LaunchURL on endpoint ID %d as it does not support "
                            "the ContentLauncher cluster",
                            endpoint->GetEndpointId());
        }
    }
}

void OnConnectionSuccess(TargetVideoPlayerInfo * videoPlayer)
{
    ChipLogProgress(AppServer,
                    "OnConnectionSuccess with Video Player(nodeId: 0x" ChipLogFormatX64
                    ", fabricIndex: %d, deviceName: %s, vendorId: %d, productId: "
                    "%d, deviceType: %d)",
                    ChipLogValueX64(videoPlayer->GetNodeId()), videoPlayer->GetFabricIndex(), videoPlayer->GetDeviceName(),
                    videoPlayer->GetVendorId(), videoPlayer->GetProductId(), videoPlayer->GetDeviceType());

    TargetEndpointInfo * endpoints = videoPlayer->GetEndpoints();
    if (endpoints != nullptr)
    {
        for (size_t i = 0; i < kMaxNumberOfEndpoints && endpoints[i].IsInitialized(); i++)
        {
            doCastingDemoActions(&endpoints[i]); // LaunchURL and Subscribe to CurrentState
        }
    }
}

void OnConnectionFailure(CHIP_ERROR err)
{
    ChipLogError(AppServer, "OnConnectionFailure error: %" CHIP_ERROR_FORMAT, err.AsString());
}

void OnNewOrUpdatedEndpoint(TargetEndpointInfo * endpoint)
{
    ChipLogProgress(AppServer, "OnNewOrUpdatedEndpoint called");
    doCastingDemoActions(endpoint); // LaunchURL and Subscribe to CurrentState
}

CHIP_ERROR ConnectToCachedVideoPlayer()
{
    TargetVideoPlayerInfo * cachedVideoPlayers = CastingServer::GetInstance()->ReadCachedTargetVideoPlayerInfos();
    if (cachedVideoPlayers != nullptr)
    {
        for (size_t i = 0; i < kMaxCachedVideoPlayers; i++)
        {
            if (cachedVideoPlayers[i].IsInitialized())
            {
                ChipLogProgress(AppServer, "Found a Cached video player with nodeId: 0x" ChipLogFormatX64 ", fabricIndex: %d",
                                ChipLogValueX64(cachedVideoPlayers[i].GetNodeId()), cachedVideoPlayers[i].GetFabricIndex());
                if (CastingServer::GetInstance()->VerifyOrEstablishConnection(
                        cachedVideoPlayers[i], OnConnectionSuccess, OnConnectionFailure, OnNewOrUpdatedEndpoint) == CHIP_NO_ERROR)
                {
                    ChipLogProgress(AppServer,
                                    "FindOrEstablish CASESession attempted for cached video player with nodeId: 0x" ChipLogFormatX64
                                    ", fabricIndex: %d",
                                    ChipLogValueX64(cachedVideoPlayers[i].GetNodeId()), cachedVideoPlayers[i].GetFabricIndex());
                    return CHIP_NO_ERROR;
                }
            }
        }
    }
    return CHIP_ERROR_INVALID_CASE_PARAMETER;
}

void HandleCommissioningCompleteCallback(CHIP_ERROR err)
{
    ChipLogProgress(AppServer, "HandleCommissioningCompleteCallback called with %" CHIP_ERROR_FORMAT, err.Format());
}

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
void HandleUDCSendExpiration(System::Layer * aSystemLayer, void * context)
{
    Dnssd::CommissionNodeData * selectedCommissioner = (Dnssd::CommissionNodeData *) context;

    // Send User Directed commissioning request
    ReturnOnFailure(CastingServer::GetInstance()->SendUserDirectedCommissioningRequest(selectedCommissioner));
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

void PrintFabrics()
{
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
        [[maybe_unused]] NodeId myNodeId = fb.GetNodeId();
        ChipLogProgress(NotSpecified,
                        "---- Current Fabric nodeId=0x" ChipLogFormatX64 " fabricId=0x" ChipLogFormatX64 " fabricIndex=%d",
                        ChipLogValueX64(myNodeId), ChipLogValueX64(fb.GetFabricId()), fabricIndex);
    }
}
