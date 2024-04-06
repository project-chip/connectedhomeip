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
#include "ConversionUtils.h"

#include "app/clusters/bindings/BindingManager.h"

using namespace chip;
using namespace chip::Controller;
using namespace chip::Credentials;
using namespace chip::app::Clusters::ContentLauncher::Commands;

CastingServer * CastingServer::castingServer_ = nullptr;

CastingServer::CastingServer() {}

CastingServer * CastingServer::GetInstance()
{
    if (castingServer_ == nullptr)
    {
        castingServer_ = new CastingServer();
    }
    return castingServer_;
}

CHIP_ERROR CastingServer::PreInit(AppParams * appParams)
{
#if CHIP_ENABLE_ROTATING_DEVICE_ID
    return SetRotatingDeviceIdUniqueId(appParams != nullptr ? appParams->GetRotatingDeviceIdUniqueId() : chip::NullOptional);
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_ENABLE_ROTATING_DEVICE_ID
}

CHIP_ERROR CastingServer::Init(AppParams * AppParams)
{
    if (mInited)
    {
        return CHIP_NO_ERROR;
    }

    // Set CastingServer as AppDelegate
    InitAppDelegation();

    // Initialize binding handlers
    ReturnErrorOnFailure(InitBindingHandlers());

    // Set FabricDelegate
    chip::Server::GetInstance().GetFabricTable().AddFabricDelegate(&mPersistenceManager);

    // Add callback to send Content casting commands after commissioning completes
    ReturnErrorOnFailure(DeviceLayer::PlatformMgrImpl().AddEventHandler(DeviceEventCallback, 0));

    mInited = true;
    return CHIP_NO_ERROR;
}

void CastingServer::InitAppDelegation()
{
    chip::Server::Server::GetInstance().GetCommissioningWindowManager().SetAppDelegate(this);
}

CHIP_ERROR CastingServer::SetRotatingDeviceIdUniqueId(chip::Optional<chip::ByteSpan> rotatingDeviceIdUniqueIdOptional)
{
#if CHIP_ENABLE_ROTATING_DEVICE_ID
    // if this class's client provided a RotatingDeviceIdUniqueId, use that
    if (rotatingDeviceIdUniqueIdOptional.HasValue())
    {
        ChipLogProgress(AppServer, "Setting rotatingDeviceIdUniqueId received from client app");
        return chip::DeviceLayer::ConfigurationMgr().SetRotatingDeviceIdUniqueId(rotatingDeviceIdUniqueIdOptional.Value());
    }
#ifdef CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID
    else
    {
        // otherwise, generate and set a random uniqueId for generating rotatingId
        ChipLogProgress(AppServer, "Setting random rotatingDeviceIdUniqueId");
        uint8_t rotatingDeviceIdUniqueId[chip::DeviceLayer::ConfigurationManager::kRotatingDeviceIDUniqueIDLength];
        for (size_t i = 0; i < sizeof(rotatingDeviceIdUniqueId); i++)
        {
            rotatingDeviceIdUniqueId[i] = chip::Crypto::GetRandU8();
        }

        return chip::DeviceLayer::ConfigurationMgr().SetRotatingDeviceIdUniqueId(ByteSpan(rotatingDeviceIdUniqueId));
    }
#endif // CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID
#endif // CHIP_ENABLE_ROTATING_DEVICE_ID
    return CHIP_NO_ERROR;
}

CHIP_ERROR CastingServer::InitBindingHandlers()
{
    auto & server = chip::Server::GetInstance();
    chip::BindingManager::GetInstance().Init(
        { &server.GetFabricTable(), server.GetCASESessionManager(), &server.GetPersistentStorage() });
    return CHIP_NO_ERROR;
}

CHIP_ERROR CastingServer::TargetVideoPlayerInfoInit(NodeId nodeId, FabricIndex fabricIndex,
                                                    std::function<void(TargetVideoPlayerInfo *)> onConnectionSuccess,
                                                    std::function<void(CHIP_ERROR)> onConnectionFailure,
                                                    std::function<void(TargetEndpointInfo *)> onNewOrUpdatedEndpoint)
{
    Init();
    mOnConnectionSuccessClientCallback = onConnectionSuccess;
    mOnConnectionFailureClientCallback = onConnectionFailure;
    mOnNewOrUpdatedEndpoint            = onNewOrUpdatedEndpoint;
    return mActiveTargetVideoPlayerInfo.Initialize(nodeId, fabricIndex, mOnConnectionSuccessClientCallback,
                                                   mOnConnectionFailureClientCallback);
}

CHIP_ERROR CastingServer::DiscoverCommissioners(DeviceDiscoveryDelegate * deviceDiscoveryDelegate)
{
    TargetVideoPlayerInfo * connectableVideoPlayerList = ReadCachedTargetVideoPlayerInfos();
    if (connectableVideoPlayerList == nullptr || !connectableVideoPlayerList[0].IsInitialized())
    {
        ChipLogProgress(AppServer, "No cached video players found during discovery");
    }

    mCommissionableNodeController.RegisterDeviceDiscoveryDelegate(deviceDiscoveryDelegate);

    // Send discover commissioners request
    return mCommissionableNodeController.DiscoverCommissioners(
        Dnssd::DiscoveryFilter(Dnssd::DiscoveryFilterType::kDeviceType, static_cast<uint16_t>(35)));
}

CHIP_ERROR CastingServer::OpenBasicCommissioningWindow(CommissioningCallbacks commissioningCallbacks,
                                                       std::function<void(TargetVideoPlayerInfo *)> onConnectionSuccess,
                                                       std::function<void(CHIP_ERROR)> onConnectionFailure,
                                                       std::function<void(TargetEndpointInfo *)> onNewOrUpdatedEndpoint)
{
    mCommissioningCallbacks            = commissioningCallbacks;
    mOnConnectionSuccessClientCallback = onConnectionSuccess;
    mOnConnectionFailureClientCallback = onConnectionFailure;
    mOnNewOrUpdatedEndpoint            = onNewOrUpdatedEndpoint;
    return Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow(kCommissioningWindowTimeout);
}

void CastingServer::OnCommissioningSessionStarted()
{
    ChipLogProgress(AppServer, "CastingServer::OnCommissioningSessionStarted");
    if (mCommissioningCallbacks.sessionEstablished)
    {

        mCommissioningCallbacks.sessionEstablished();
    }
}

void CastingServer::OnCommissioningSessionEstablishmentError(CHIP_ERROR err)
{
    ChipLogProgress(AppServer, "CastingServer::OnCommissioningSessionEstablishmentError");
    if (mCommissioningCallbacks.sessionEstablishmentError)
    {
        mCommissioningCallbacks.sessionEstablishmentError(err);
    }
}

void CastingServer::OnCommissioningSessionStopped()
{
    ChipLogProgress(AppServer, "CastingServer::OnCommissioningSessionStopped");
    if (mCommissioningCallbacks.sessionEstablishmentStopped)
    {
        mCommissioningCallbacks.sessionEstablishmentStopped();
    }
}

void CastingServer::OnCommissioningSessionEstablishmentStarted()
{
    ChipLogProgress(AppServer, "CastingServer::OnCommissioningSessionEstablishmentStarted");
    if (mCommissioningCallbacks.sessionEstablishmentStarted)
    {
        mCommissioningCallbacks.sessionEstablishmentStarted();
    }
}

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
CHIP_ERROR CastingServer::SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress commissioner)
{
    // TODO: expose options to the higher layer
    Protocols::UserDirectedCommissioning::IdentificationDeclaration id;
    return Server::GetInstance().SendUserDirectedCommissioningRequest(commissioner, id);
}

chip::Inet::IPAddress * CastingServer::getIpAddressForUDCRequest(chip::Inet::IPAddress ipAddresses[], const size_t numIPs)
{
    size_t ipIndexToUse = 0;
    for (size_t i = 0; i < numIPs; i++)
    {
        if (ipAddresses[i].IsIPv4())
        {
            ipIndexToUse = i;
            ChipLogProgress(AppServer, "Found IPv4 address at index: %lu - prioritizing use of IPv4",
                            static_cast<long>(ipIndexToUse));
            break;
        }

        if (i == (numIPs - 1))
        {
            ChipLogProgress(AppServer, "Could not find an IPv4 address, defaulting to the first address in IP list");
        }
    }

    return &ipAddresses[ipIndexToUse];
}

CHIP_ERROR CastingServer::SendUserDirectedCommissioningRequest(Dnssd::DiscoveredNodeData * selectedCommissioner)
{
    mUdcInProgress = true;
    // Send User Directed commissioning request
    chip::Inet::IPAddress * ipAddressToUse =
        getIpAddressForUDCRequest(selectedCommissioner->resolutionData.ipAddress, selectedCommissioner->resolutionData.numIPs);
    ReturnErrorOnFailure(SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress::UDP(
        *ipAddressToUse, selectedCommissioner->resolutionData.port, selectedCommissioner->resolutionData.interfaceId)));
    mTargetVideoPlayerVendorId   = selectedCommissioner->commissionData.vendorId;
    mTargetVideoPlayerProductId  = selectedCommissioner->commissionData.productId;
    mTargetVideoPlayerDeviceType = selectedCommissioner->commissionData.deviceType;
    mTargetVideoPlayerNumIPs     = selectedCommissioner->resolutionData.numIPs;
    for (size_t i = 0; i < mTargetVideoPlayerNumIPs && i < chip::Dnssd::CommonResolutionData::kMaxIPAddresses; i++)
    {
        mTargetVideoPlayerIpAddress[i] = selectedCommissioner->resolutionData.ipAddress[i];
    }
    chip::Platform::CopyString(mTargetVideoPlayerDeviceName, chip::Dnssd::kMaxDeviceNameLen + 1,
                               selectedCommissioner->commissionData.deviceName);
    chip::Platform::CopyString(mTargetVideoPlayerHostName, chip::Dnssd::kHostNameMaxLength + 1,
                               selectedCommissioner->resolutionData.hostName);
    chip::Platform::CopyString(mTargetVideoPlayerInstanceName, chip::Dnssd::Commission::kInstanceNameMaxLength + 1,
                               selectedCommissioner->commissionData.instanceName);
    mTargetVideoPlayerPort = selectedCommissioner->resolutionData.port;
    return CHIP_NO_ERROR;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

const Dnssd::DiscoveredNodeData *
CastingServer::GetDiscoveredCommissioner(int index, chip::Optional<TargetVideoPlayerInfo *> & outAssociatedConnectableVideoPlayer)
{
    const Dnssd::DiscoveredNodeData * discoveredNodeData = mCommissionableNodeController.GetDiscoveredCommissioner(index);
    if (discoveredNodeData != nullptr)
    {
        for (size_t i = 0; i < kMaxCachedVideoPlayers && mCachedTargetVideoPlayerInfo[i].IsInitialized(); i++)
        {
            if (mCachedTargetVideoPlayerInfo[i].IsSameAs(discoveredNodeData))
            {
                outAssociatedConnectableVideoPlayer = MakeOptional(&mCachedTargetVideoPlayerInfo[i]);
            }
        }
    }
    return discoveredNodeData;
}

void CastingServer::ReadServerClustersForNode(NodeId nodeId)
{
    ChipLogProgress(NotSpecified, "ReadServerClustersForNode nodeId=0x" ChipLogFormatX64, ChipLogValueX64(nodeId));
    CHIP_ERROR err = Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Init error: %" CHIP_ERROR_FORMAT, err.Format());
    }

    for (const auto & binding : BindingTable::GetInstance())
    {
        ChipLogProgress(NotSpecified,
                        "Binding type=%d fab=%d nodeId=0x" ChipLogFormatX64
                        " groupId=%d local endpoint=%d remote endpoint=%d cluster=" ChipLogFormatMEI,
                        binding.type, binding.fabricIndex, ChipLogValueX64(binding.nodeId), binding.groupId, binding.local,
                        binding.remote, ChipLogValueMEI(binding.clusterId.ValueOr(0)));
        if (binding.type == MATTER_UNICAST_BINDING && nodeId == binding.nodeId)
        {
            if (!mActiveTargetVideoPlayerInfo.HasEndpoint(binding.remote))
            {
                ReadServerClusters(binding.remote);
            }
            else
            {
                TargetEndpointInfo * endpointInfo = mActiveTargetVideoPlayerInfo.GetEndpoint(binding.remote);
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
    const OperationalDeviceProxy * deviceProxy = mActiveTargetVideoPlayerInfo.GetOperationalDeviceProxy();
    if (deviceProxy == nullptr)
    {
        ChipLogError(AppServer, "Failed in getting an instance of DeviceProxy");
        return;
    }

    // GetOperationalDeviceProxy only passes us a deviceProxy if we can get a SessionHandle.
    chip::Controller::ClusterBase cluster(*deviceProxy->GetExchangeManager(), deviceProxy->GetSecureSession().Value(), endpointId);

    TargetEndpointInfo * endpointInfo = mActiveTargetVideoPlayerInfo.GetOrAddEndpoint(endpointId);

    if (cluster.ReadAttribute<app::Clusters::Descriptor::Attributes::ServerList::TypeInfo>(
            endpointInfo, CastingServer::OnDescriptorReadSuccessResponse, CastingServer::OnDescriptorReadFailureResponse) !=
        CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Could not read Descriptor cluster ServerList");
    }

    ChipLogProgress(Controller, "Sent descriptor read for remote endpoint=%d", endpointId);
}

CHIP_ERROR CastingServer::SendWakeOnLan(TargetVideoPlayerInfo & targetVideoPlayerInfo)
{
    return SendWakeOnLanPacket(targetVideoPlayerInfo.GetMACAddress());
}

CHIP_ERROR CastingServer::ReadMACAddress(TargetEndpointInfo * endpoint)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (endpoint != nullptr && endpoint->HasCluster(chip::app::Clusters::WakeOnLan::Id))
    {
        // Read MAC address
        ChipLogProgress(AppServer, "Endpoint supports WoL. Reading Active VideoPlayer's MACAddress");
        mMACAddressReader.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId());
        err = mMACAddressReader.ReadAttribute(
            &mActiveTargetVideoPlayerInfo,
            [](void * context, const chip::app::Clusters::WakeOnLan::Attributes::MACAddress::TypeInfo::DecodableArgType response) {
                ChipLogProgress(AppServer, "Read MACAddress successfully");
                TargetVideoPlayerInfo * videoPlayerInfo = static_cast<TargetVideoPlayerInfo *>(context);
                if (response.data() != nullptr && response.size() > 0)
                {
                    videoPlayerInfo->SetMACAddress(response);
                    ChipLogProgress(AppServer, "Updating cache of VideoPlayers with MACAddress: %.*s",
                                    static_cast<int>(response.size()), response.data());
                    CHIP_ERROR error = CastingServer::GetInstance()->mPersistenceManager.AddVideoPlayer(videoPlayerInfo);
                    if (error != CHIP_NO_ERROR)
                    {
                        ChipLogError(AppServer, "AddVideoPlayer(ToCache) error: %" CHIP_ERROR_FORMAT, error.Format());
                    }
                }
            },
            [](void * context, CHIP_ERROR error) { ChipLogError(AppServer, "Failed to read MACAddress"); });
    }
    else
    {
        err = CHIP_ERROR_INVALID_ARGUMENT;
    }
    return err;
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
    CastingServer::GetInstance()->mActiveTargetVideoPlayerInfo.PrintInfo();

    CHIP_ERROR err = CastingServer::GetInstance()->mPersistenceManager.AddVideoPlayer(
        &CastingServer::GetInstance()->mActiveTargetVideoPlayerInfo);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "AddVideoPlayer(ToCache) error: %" CHIP_ERROR_FORMAT, err.Format());
    }

    // Read WoL:MACAddress (if available from this endpoint)
    CastingServer::GetInstance()->ReadMACAddress(endpointInfo);

    if (CastingServer::GetInstance()->mOnNewOrUpdatedEndpoint)
    {
        CastingServer::GetInstance()->mOnNewOrUpdatedEndpoint(endpointInfo);
    }
}

void CastingServer::OnDescriptorReadFailureResponse(void * context, CHIP_ERROR error)
{
    TargetEndpointInfo * endpointInfo = static_cast<TargetEndpointInfo *>(context);
    ChipLogError(AppServer, "Descriptor: Default Failure Response: %" CHIP_ERROR_FORMAT, error.Format());

    CHIP_ERROR err = CastingServer::GetInstance()->mPersistenceManager.AddVideoPlayer(
        &CastingServer::GetInstance()->mActiveTargetVideoPlayerInfo);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "AddVideoPlayer(ToCache) error: %" CHIP_ERROR_FORMAT, err.Format());
    }

    if (CastingServer::GetInstance()->mOnNewOrUpdatedEndpoint)
    {
        CastingServer::GetInstance()->mOnNewOrUpdatedEndpoint(endpointInfo);
    }
}

TargetVideoPlayerInfo * CastingServer::ReadCachedTargetVideoPlayerInfos()
{
    CHIP_ERROR err = mPersistenceManager.ReadAllVideoPlayers(mCachedTargetVideoPlayerInfo);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(AppServer, "ReadAllVideoPlayers error: %" CHIP_ERROR_FORMAT, err.Format());
        return nullptr;
    }
    return mCachedTargetVideoPlayerInfo;
}

void CastingServer::LogCachedVideoPlayers()
{
    ChipLogProgress(AppServer, "CastingServer:LogCachedVideoPlayers dumping any/all cached video players.");
    for (size_t i = 0; i < kMaxCachedVideoPlayers && mCachedTargetVideoPlayerInfo[i].IsInitialized(); i++)
    {
        mCachedTargetVideoPlayerInfo[i].PrintInfo();
    }
}

CHIP_ERROR CastingServer::VerifyOrEstablishConnection(TargetVideoPlayerInfo & targetVideoPlayerInfo,
                                                      std::function<void(TargetVideoPlayerInfo *)> onConnectionSuccess,
                                                      std::function<void(CHIP_ERROR)> onConnectionFailure,
                                                      std::function<void(TargetEndpointInfo *)> onNewOrUpdatedEndpoint)
{
    LogCachedVideoPlayers();

    if (!targetVideoPlayerInfo.IsInitialized())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    mOnConnectionSuccessClientCallback = onConnectionSuccess;
    mOnConnectionFailureClientCallback = onConnectionFailure;
    mOnNewOrUpdatedEndpoint            = onNewOrUpdatedEndpoint;

    chip::OperationalDeviceProxy * prevDeviceProxy =
        CastingServer::GetInstance()->mActiveTargetVideoPlayerInfo.GetOperationalDeviceProxy();
    if (prevDeviceProxy != nullptr)
    {
        ChipLogProgress(AppServer, "CastingServer::VerifyOrEstablishConnection Disconnecting previous deviceProxy");
        prevDeviceProxy->Disconnect();
    }

    CastingServer::GetInstance()->mActiveTargetVideoPlayerInfo = targetVideoPlayerInfo;
    uint32_t delay                                             = 0;
    if (targetVideoPlayerInfo.IsAsleep())
    {
        ChipLogProgress(AppServer, "CastingServer::VerifyOrEstablishConnection(): Sending WoL to sleeping VideoPlayer and waiting");
        ReturnErrorOnFailure(CastingServer::GetInstance()->SendWakeOnLan(targetVideoPlayerInfo));

#ifdef CHIP_DEVICE_CONFIG_STR_WAKE_UP_DELAY_SEC
        delay = CHIP_DEVICE_CONFIG_STR_WAKE_UP_DELAY_SEC * 1000;
#endif
    }

    // cancel preexisting timer for VerifyOrEstablishConnectionTask, if any, and schedule the task
    chip::DeviceLayer::SystemLayer().CancelTimer(VerifyOrEstablishConnectionTask, nullptr);
    return chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(delay), VerifyOrEstablishConnectionTask,
                                                       nullptr);
}

void CastingServer::VerifyOrEstablishConnectionTask(chip::System::Layer * aSystemLayer, void * context)
{
    ChipLogProgress(AppServer, "CastingServer::VerifyOrEstablishConnectionTask called");
    CastingServer::GetInstance()->mActiveTargetVideoPlayerInfo.PrintInfo();
    CHIP_ERROR err = CastingServer::GetInstance()->mActiveTargetVideoPlayerInfo.FindOrEstablishCASESession(
        [](TargetVideoPlayerInfo * videoPlayer) {
            ChipLogProgress(AppServer, "CastingServer::OnConnectionSuccess lambda called");
            CastingServer::GetInstance()->mActiveTargetVideoPlayerInfo = *videoPlayer;
            CastingServer::GetInstance()->ReadMACAddress(
                videoPlayer->GetEndpoint(1)); // Read MACAddress from cached VideoPlayer endpoint (1) which supports WoL
            CastingServer::GetInstance()->mOnConnectionSuccessClientCallback(videoPlayer);
        },
        [](CHIP_ERROR error) {
            ChipLogProgress(AppServer, "Deleting VideoPlayer from cache after connection failure: %" CHIP_ERROR_FORMAT,
                            error.Format());
            CastingServer::GetInstance()->mPersistenceManager.DeleteVideoPlayer(
                &CastingServer::GetInstance()->mActiveTargetVideoPlayerInfo);
            CastingServer::GetInstance()->mOnConnectionFailureClientCallback(error);
        });
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer,
                     "CastingServer::VerifyOrEstablishConnectionTask FindOrEstablishCASESession failed with %" CHIP_ERROR_FORMAT,
                     err.Format());
        CastingServer::GetInstance()->mOnConnectionFailureClientCallback(err);
    }
}

CHIP_ERROR CastingServer::PurgeCache()
{
    return mPersistenceManager.PurgeVideoPlayerCache();
}

CHIP_ERROR CastingServer::AddVideoPlayer(TargetVideoPlayerInfo * targetVideoPlayerInfo)
{
    return CastingServer::GetInstance()->mPersistenceManager.AddVideoPlayer(targetVideoPlayerInfo);
}

[[deprecated("Use ContentLauncher_LaunchURL(..) instead")]] CHIP_ERROR
CastingServer::ContentLauncherLaunchURL(TargetEndpointInfo * endpoint, const char * contentUrl, const char * contentDisplayStr,
                                        std::function<void(CHIP_ERROR)> launchURLResponseCallback)
{
    return ContentLauncher_LaunchURL(endpoint, contentUrl, contentDisplayStr,
                                     MakeOptional(chip::app::Clusters::ContentLauncher::Structs::BrandingInformationStruct::Type()),
                                     launchURLResponseCallback);
}

void CastingServer::DeviceEventCallback(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    bool runPostCommissioning           = false;
    chip::NodeId targetPeerNodeId       = 0;
    chip::FabricIndex targetFabricIndex = 0;
    if (event->Type == DeviceLayer::DeviceEventType::kBindingsChangedViaCluster)
    {
        ChipLogProgress(AppServer, "CastingServer::DeviceEventCallback kBindingsChangedViaCluster received");
        if (CastingServer::GetInstance()->GetActiveTargetVideoPlayer()->IsInitialized() &&
            CastingServer::GetInstance()->GetActiveTargetVideoPlayer()->GetOperationalDeviceProxy() != nullptr)
        {
            ChipLogProgress(AppServer,
                            "CastingServer::DeviceEventCallback already connected to video player, reading server clusters");
            CastingServer::GetInstance()->ReadServerClustersForNode(
                CastingServer::GetInstance()->GetActiveTargetVideoPlayer()->GetNodeId());
        }
        else if (CastingServer::GetInstance()->mUdcInProgress)
        {
            ChipLogProgress(AppServer,
                            "CastingServer::DeviceEventCallback UDC is in progress while handling kBindingsChangedViaCluster with "
                            "fabricIndex: %d",
                            event->BindingsChanged.fabricIndex);
            CastingServer::GetInstance()->mUdcInProgress = false;

            // find targetPeerNodeId from binding table by matching the binding's fabricIndex with the accessing fabricIndex
            // received in BindingsChanged event
            for (const auto & binding : BindingTable::GetInstance())
            {
                ChipLogProgress(
                    AppServer,
                    "CastingServer::DeviceEventCallback Read cached binding type=%d fabrixIndex=%d nodeId=0x" ChipLogFormatX64
                    " groupId=%d local endpoint=%d remote endpoint=%d cluster=" ChipLogFormatMEI,
                    binding.type, binding.fabricIndex, ChipLogValueX64(binding.nodeId), binding.groupId, binding.local,
                    binding.remote, ChipLogValueMEI(binding.clusterId.ValueOr(0)));
                if (binding.type == MATTER_UNICAST_BINDING && event->BindingsChanged.fabricIndex == binding.fabricIndex)
                {
                    ChipLogProgress(
                        NotSpecified,
                        "CastingServer::DeviceEventCallback Matched accessingFabricIndex with nodeId=0x" ChipLogFormatX64,
                        ChipLogValueX64(binding.nodeId));
                    targetPeerNodeId     = binding.nodeId;
                    targetFabricIndex    = binding.fabricIndex;
                    runPostCommissioning = true;
                    break;
                }
            }

            if (targetPeerNodeId == 0 && runPostCommissioning == false)
            {
                ChipLogError(AppServer, "CastingServer::DeviceEventCallback accessingFabricIndex: %d did not match bindings",
                             event->BindingsChanged.fabricIndex);
                if (CastingServer::GetInstance()->mCommissioningCallbacks.commissioningComplete)
                {
                    CastingServer::GetInstance()->mCommissioningCallbacks.commissioningComplete(CHIP_ERROR_INCORRECT_STATE);
                }
                return;
            }
        }
    }
    else if (event->Type == DeviceLayer::DeviceEventType::kCommissioningComplete)
    {
        ChipLogProgress(AppServer, "CastingServer::DeviceEventCallback kCommissioningComplete received");
        CastingServer::GetInstance()->mUdcInProgress = false;
        targetPeerNodeId                             = event->CommissioningComplete.nodeId;
        targetFabricIndex                            = event->CommissioningComplete.fabricIndex;
        runPostCommissioning                         = true;
    }

    if (runPostCommissioning)
    {
        ChipLogProgress(AppServer,
                        "CastingServer::DeviceEventCallback will connect with nodeId=0x" ChipLogFormatX64 " fabricIndex=%d",
                        ChipLogValueX64(targetPeerNodeId), targetFabricIndex);
        CHIP_ERROR err = CastingServer::GetInstance()->GetActiveTargetVideoPlayer()->Initialize(
            targetPeerNodeId, targetFabricIndex, CastingServer::GetInstance()->mOnConnectionSuccessClientCallback,
            CastingServer::GetInstance()->mOnConnectionFailureClientCallback,
            CastingServer::GetInstance()->mTargetVideoPlayerVendorId, CastingServer::GetInstance()->mTargetVideoPlayerProductId,
            CastingServer::GetInstance()->mTargetVideoPlayerDeviceType, CastingServer::GetInstance()->mTargetVideoPlayerDeviceName,
            CastingServer::GetInstance()->mTargetVideoPlayerHostName, CastingServer::GetInstance()->mTargetVideoPlayerNumIPs,
            CastingServer::GetInstance()->mTargetVideoPlayerIpAddress, CastingServer::GetInstance()->mTargetVideoPlayerPort,
            CastingServer::GetInstance()->mTargetVideoPlayerInstanceName, System::SystemClock().GetMonotonicMilliseconds64());

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Failed to initialize target video player");
        }
        else
        {
            // add discovery timestamp
            chip::System::Clock::Timestamp currentUnixTimeMS = chip::System::Clock::kZero;
            chip::System::SystemClock().GetClock_RealTimeMS(currentUnixTimeMS);
            ChipLogProgress(AppServer, "Updating discovery timestamp for VideoPlayer to %lu",
                            static_cast<unsigned long>(currentUnixTimeMS.count()));
            CastingServer::GetInstance()->mActiveTargetVideoPlayerInfo.SetLastDiscovered(currentUnixTimeMS);
        }

        err = CastingServer::GetInstance()->mPersistenceManager.AddVideoPlayer(
            &CastingServer::GetInstance()->mActiveTargetVideoPlayerInfo);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "AddVideoPlayer(ToCache) error: %" CHIP_ERROR_FORMAT, err.Format());
        }

        if (CastingServer::GetInstance()->mCommissioningCallbacks.commissioningComplete)
        {
            CastingServer::GetInstance()->mCommissioningCallbacks.commissioningComplete(err);
        }
    }
}

// given a fabric index, try to determine the video-player nodeId by searching the binding table
NodeId CastingServer::GetVideoPlayerNodeForFabricIndex(FabricIndex fabricIndex)
{
    CHIP_ERROR err = Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "GetVideoPlayerNodeForFabricIndex Init error: %" CHIP_ERROR_FORMAT, err.Format());
    }
    for (const auto & binding : BindingTable::GetInstance())
    {
        ChipLogProgress(NotSpecified,
                        "Binding type=%d fab=%d nodeId=0x" ChipLogFormatX64
                        " groupId=%d local endpoint=%d remote endpoint=%d cluster=" ChipLogFormatMEI,
                        binding.type, binding.fabricIndex, ChipLogValueX64(binding.nodeId), binding.groupId, binding.local,
                        binding.remote, ChipLogValueMEI(binding.clusterId.ValueOr(0)));
        if (binding.type == MATTER_UNICAST_BINDING && fabricIndex == binding.fabricIndex)
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
    CHIP_ERROR err = Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "GetVideoPlayerFabricIndexForNode Init error: %" CHIP_ERROR_FORMAT, err.Format());
    }
    for (const auto & binding : BindingTable::GetInstance())
    {
        ChipLogProgress(NotSpecified,
                        "Binding type=%d fab=%d nodeId=0x" ChipLogFormatX64
                        " groupId=%d local endpoint=%d remote endpoint=%d cluster=" ChipLogFormatMEI,
                        binding.type, binding.fabricIndex, ChipLogValueX64(binding.nodeId), binding.groupId, binding.local,
                        binding.remote, ChipLogValueMEI(binding.clusterId.ValueOr(0)));
        if (binding.type == MATTER_UNICAST_BINDING && nodeId == binding.nodeId)
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

void CastingServer::SetDefaultFabricIndex(std::function<void(TargetVideoPlayerInfo *)> onConnectionSuccess,
                                          std::function<void(CHIP_ERROR)> onConnectionFailure,
                                          std::function<void(TargetEndpointInfo *)> onNewOrUpdatedEndpoint)
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
        [[maybe_unused]] NodeId myNodeId = fb.GetNodeId();
        ChipLogProgress(NotSpecified,
                        "---- Current Fabric nodeId=0x" ChipLogFormatX64 " fabricId=0x" ChipLogFormatX64 " fabricIndex=%d",
                        ChipLogValueX64(myNodeId), ChipLogValueX64(fb.GetFabricId()), fabricIndex);

        NodeId videoPlayerNodeId = GetVideoPlayerNodeForFabricIndex(fabricIndex);
        if (videoPlayerNodeId == kUndefinedNodeId)
        {
            // could not determine video player nodeid for this fabric
            continue;
        }

        mOnConnectionSuccessClientCallback = onConnectionSuccess;
        mOnConnectionFailureClientCallback = onConnectionFailure;
        mOnNewOrUpdatedEndpoint            = onNewOrUpdatedEndpoint;

        mActiveTargetVideoPlayerInfo.Initialize(videoPlayerNodeId, fabricIndex, mOnConnectionSuccessClientCallback,
                                                mOnConnectionFailureClientCallback);
        return;
    }
    ChipLogError(AppServer, " -- No initialized fabrics with video players");
}

void CastingServer::ShutdownAllSubscriptions()
{
    ChipLogProgress(AppServer, "Shutting down ALL Subscriptions");
    app::InteractionModelEngine::GetInstance()->ShutdownAllSubscriptions();
}

void CastingServer::Disconnect()
{
    TargetVideoPlayerInfo * currentVideoPlayer = GetActiveTargetVideoPlayer();

    if (currentVideoPlayer != nullptr && currentVideoPlayer->IsInitialized())
    {
        chip::OperationalDeviceProxy * operationalDeviceProxy = currentVideoPlayer->GetOperationalDeviceProxy();
        if (operationalDeviceProxy != nullptr)
        {
            ChipLogProgress(AppServer, "Disconnecting from VideoPlayer with nodeId=0x" ChipLogFormatX64 " fabricIndex=%d",
                            ChipLogValueX64(currentVideoPlayer->GetNodeId()), currentVideoPlayer->GetFabricIndex());
            operationalDeviceProxy->Disconnect();
        }
    }
}

/**
 * @brief Content Launcher cluster
 */
CHIP_ERROR CastingServer::ContentLauncher_LaunchURL(
    TargetEndpointInfo * endpoint, const char * contentUrl, const char * contentDisplayStr,
    chip::Optional<chip::app::Clusters::ContentLauncher::Structs::BrandingInformationStruct::Type> brandingInformation,
    std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mLaunchURLCommand.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mLaunchURLCommand.Invoke(contentUrl, contentDisplayStr, brandingInformation, responseCallback);
}

CHIP_ERROR CastingServer::ContentLauncher_LaunchContent(
    TargetEndpointInfo * endpoint, chip::app::Clusters::ContentLauncher::Structs::ContentSearchStruct::Type search, bool autoPlay,
    chip::Optional<chip::CharSpan> data, std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mLaunchContentCommand.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mLaunchContentCommand.Invoke(search, autoPlay, data, responseCallback);
}

CHIP_ERROR
CastingServer::ContentLauncher_SubscribeToAcceptHeader(
    TargetEndpointInfo * endpoint, void * context,
    chip::Controller::ReadResponseSuccessCallback<
        chip::app::Clusters::ContentLauncher::Attributes::AcceptHeader::TypeInfo::DecodableArgType>
        successFn,
    chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
    chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished)
{
    ReturnErrorOnFailure(mAcceptHeaderSubscriber.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mAcceptHeaderSubscriber.SubscribeAttribute(context, successFn, failureFn, minInterval, maxInterval,
                                                      onSubscriptionEstablished);
}

CHIP_ERROR
CastingServer::ContentLauncher_SubscribeToSupportedStreamingProtocols(
    TargetEndpointInfo * endpoint, void * context,
    chip::Controller::ReadResponseSuccessCallback<
        chip::app::Clusters::ContentLauncher::Attributes::SupportedStreamingProtocols::TypeInfo::DecodableArgType>
        successFn,
    chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
    chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished)
{
    ReturnErrorOnFailure(mSupportedStreamingProtocolsSubscriber.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mSupportedStreamingProtocolsSubscriber.SubscribeAttribute(context, successFn, failureFn, minInterval, maxInterval,
                                                                     onSubscriptionEstablished);
}

/**
 * @brief Level Control cluster
 */
CHIP_ERROR CastingServer::LevelControl_Step(TargetEndpointInfo * endpoint, chip::app::Clusters::LevelControl::StepModeEnum stepMode,
                                            uint8_t stepSize, uint16_t transitionTime, uint8_t optionMask, uint8_t optionOverride,
                                            std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mStepCommand.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));

    app::DataModel::Nullable<uint16_t> nullableTransitionTime;
    nullableTransitionTime.SetNonNull(transitionTime);

    return mStepCommand.Invoke(stepMode, stepSize, nullableTransitionTime, optionMask, optionOverride, responseCallback);
}

CHIP_ERROR CastingServer::LevelControl_MoveToLevel(TargetEndpointInfo * endpoint, uint8_t level, uint16_t transitionTime,
                                                   uint8_t optionMask, uint8_t optionOverride,
                                                   std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mMoveToLevelCommand.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));

    app::DataModel::Nullable<uint16_t> nullableTransitionTime;
    nullableTransitionTime.SetNonNull(transitionTime);

    return mMoveToLevelCommand.Invoke(level, nullableTransitionTime, optionMask, optionOverride, responseCallback);
}

CHIP_ERROR CastingServer::LevelControl_SubscribeToCurrentLevel(
    TargetEndpointInfo * endpoint, void * context,
    ReadResponseSuccessCallback<chip::app::Clusters::LevelControl::Attributes::CurrentLevel::TypeInfo::DecodableArgType> successFn,
    ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
    SubscriptionEstablishedCallback onSubscriptionEstablished)
{
    ReturnErrorOnFailure(mCurrentLevelSubscriber.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mCurrentLevelSubscriber.SubscribeAttribute(context, successFn, failureFn, minInterval, maxInterval,
                                                      onSubscriptionEstablished);
}

CHIP_ERROR CastingServer::LevelControl_SubscribeToMinLevel(
    TargetEndpointInfo * endpoint, void * context,
    ReadResponseSuccessCallback<chip::app::Clusters::LevelControl::Attributes::MinLevel::TypeInfo::DecodableArgType> successFn,
    ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
    SubscriptionEstablishedCallback onSubscriptionEstablished)
{
    ReturnErrorOnFailure(mMinLevelSubscriber.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mMinLevelSubscriber.SubscribeAttribute(context, successFn, failureFn, minInterval, maxInterval,
                                                  onSubscriptionEstablished);
}

CHIP_ERROR CastingServer::LevelControl_SubscribeToMaxLevel(
    TargetEndpointInfo * endpoint, void * context,
    ReadResponseSuccessCallback<chip::app::Clusters::LevelControl::Attributes::MaxLevel::TypeInfo::DecodableArgType> successFn,
    ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
    SubscriptionEstablishedCallback onSubscriptionEstablished)
{
    ReturnErrorOnFailure(mMaxLevelSubscriber.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mMaxLevelSubscriber.SubscribeAttribute(context, successFn, failureFn, minInterval, maxInterval,
                                                  onSubscriptionEstablished);
}

/**
 * @brief OnOff cluster
 */
CHIP_ERROR CastingServer::OnOff_On(TargetEndpointInfo * endpoint, std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mOnCommand.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mOnCommand.Invoke(responseCallback);
}

CHIP_ERROR CastingServer::OnOff_Off(TargetEndpointInfo * endpoint, std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mOffCommand.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mOffCommand.Invoke(responseCallback);
}

CHIP_ERROR CastingServer::OnOff_Toggle(TargetEndpointInfo * endpoint, std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mToggleCommand.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mToggleCommand.Invoke(responseCallback);
}

/**
 * @brief Messages cluster
 */
CHIP_ERROR CastingServer::Messages_PresentMessagesRequest(TargetEndpointInfo * endpoint, const char * messageText,
                                                          std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mPresentMessagesRequestCommand.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mPresentMessagesRequestCommand.Invoke(messageText, responseCallback);
}

/**
 * @brief Media Playback cluster
 */
CHIP_ERROR CastingServer::MediaPlayback_Play(TargetEndpointInfo * endpoint, std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mPlayCommand.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mPlayCommand.Invoke(responseCallback);
}

CHIP_ERROR CastingServer::MediaPlayback_Pause(TargetEndpointInfo * endpoint, std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mPauseCommand.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mPauseCommand.Invoke(responseCallback);
}

CHIP_ERROR CastingServer::MediaPlayback_StopPlayback(TargetEndpointInfo * endpoint,
                                                     std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mStopPlaybackCommand.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mStopPlaybackCommand.Invoke(responseCallback);
}

CHIP_ERROR CastingServer::MediaPlayback_Next(TargetEndpointInfo * endpoint, std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mNextCommand.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mNextCommand.Invoke(responseCallback);
}

CHIP_ERROR CastingServer::MediaPlayback_Previous(TargetEndpointInfo * endpoint, std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mPreviousCommand.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mPreviousCommand.Invoke(responseCallback);
}

CHIP_ERROR CastingServer::MediaPlayback_Rewind(TargetEndpointInfo * endpoint, std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mRewindCommand.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mRewindCommand.Invoke(responseCallback);
}

CHIP_ERROR CastingServer::MediaPlayback_FastForward(TargetEndpointInfo * endpoint, std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mFastForwardCommand.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mFastForwardCommand.Invoke(responseCallback);
}

CHIP_ERROR CastingServer::MediaPlayback_StartOver(TargetEndpointInfo * endpoint, std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mStartOverCommand.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mStartOverCommand.Invoke(responseCallback);
}

CHIP_ERROR CastingServer::MediaPlayback_Seek(TargetEndpointInfo * endpoint, uint64_t position,
                                             std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mSeekCommand.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mSeekCommand.Invoke(position, responseCallback);
}

CHIP_ERROR CastingServer::MediaPlayback_SkipForward(TargetEndpointInfo * endpoint, uint64_t deltaPositionMilliseconds,
                                                    std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mSkipForwardCommand.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mSkipForwardCommand.Invoke(deltaPositionMilliseconds, responseCallback);
}

CHIP_ERROR CastingServer::MediaPlayback_SkipBackward(TargetEndpointInfo * endpoint, uint64_t deltaPositionMilliseconds,
                                                     std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mSkipBackwardCommand.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mSkipBackwardCommand.Invoke(deltaPositionMilliseconds, responseCallback);
}

CHIP_ERROR CastingServer::MediaPlayback_SubscribeToCurrentState(
    TargetEndpointInfo * endpoint, void * context,
    ReadResponseSuccessCallback<chip::app::Clusters::MediaPlayback::Attributes::CurrentState::TypeInfo::DecodableArgType> successFn,
    ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
    SubscriptionEstablishedCallback onSubscriptionEstablished)
{
    ReturnErrorOnFailure(mCurrentStateSubscriber.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mCurrentStateSubscriber.SubscribeAttribute(context, successFn, failureFn, minInterval, maxInterval,
                                                      onSubscriptionEstablished);
}

CHIP_ERROR CastingServer::MediaPlayback_SubscribeToStartTime(
    TargetEndpointInfo * endpoint, void * context,
    ReadResponseSuccessCallback<chip::app::Clusters::MediaPlayback::Attributes::StartTime::TypeInfo::DecodableArgType> successFn,
    ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
    SubscriptionEstablishedCallback onSubscriptionEstablished)
{
    ReturnErrorOnFailure(mStartTimeSubscriber.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mStartTimeSubscriber.SubscribeAttribute(context, successFn, failureFn, minInterval, maxInterval,
                                                   onSubscriptionEstablished);
}

CHIP_ERROR CastingServer::MediaPlayback_SubscribeToDuration(
    TargetEndpointInfo * endpoint, void * context,
    ReadResponseSuccessCallback<chip::app::Clusters::MediaPlayback::Attributes::Duration::TypeInfo::DecodableArgType> successFn,
    ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
    SubscriptionEstablishedCallback onSubscriptionEstablished)
{
    ReturnErrorOnFailure(mDurationSubscriber.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mDurationSubscriber.SubscribeAttribute(context, successFn, failureFn, minInterval, maxInterval,
                                                  onSubscriptionEstablished);
}

CHIP_ERROR CastingServer::MediaPlayback_SubscribeToSampledPosition(
    TargetEndpointInfo * endpoint, void * context,
    ReadResponseSuccessCallback<chip::app::Clusters::MediaPlayback::Attributes::SampledPosition::TypeInfo::DecodableArgType>
        successFn,
    ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
    SubscriptionEstablishedCallback onSubscriptionEstablished)
{
    ReturnErrorOnFailure(mSampledPositionSubscriber.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mSampledPositionSubscriber.SubscribeAttribute(context, successFn, failureFn, minInterval, maxInterval,
                                                         onSubscriptionEstablished);
}

CHIP_ERROR CastingServer::MediaPlayback_SubscribeToPlaybackSpeed(
    TargetEndpointInfo * endpoint, void * context,
    ReadResponseSuccessCallback<chip::app::Clusters::MediaPlayback::Attributes::PlaybackSpeed::TypeInfo::DecodableArgType>
        successFn,
    ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
    SubscriptionEstablishedCallback onSubscriptionEstablished)
{
    ReturnErrorOnFailure(mPlaybackSpeedSubscriber.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mPlaybackSpeedSubscriber.SubscribeAttribute(context, successFn, failureFn, minInterval, maxInterval,
                                                       onSubscriptionEstablished);
}

CHIP_ERROR CastingServer::MediaPlayback_SubscribeToSeekRangeEnd(
    TargetEndpointInfo * endpoint, void * context,
    ReadResponseSuccessCallback<chip::app::Clusters::MediaPlayback::Attributes::SeekRangeEnd::TypeInfo::DecodableArgType> successFn,
    ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
    SubscriptionEstablishedCallback onSubscriptionEstablished)
{
    ReturnErrorOnFailure(mSeekRangeEndSubscriber.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mSeekRangeEndSubscriber.SubscribeAttribute(context, successFn, failureFn, minInterval, maxInterval,
                                                      onSubscriptionEstablished);
}

CHIP_ERROR CastingServer::MediaPlayback_SubscribeToSeekRangeStart(
    TargetEndpointInfo * endpoint, void * context,
    ReadResponseSuccessCallback<chip::app::Clusters::MediaPlayback::Attributes::SeekRangeStart::TypeInfo::DecodableArgType>
        successFn,
    ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
    SubscriptionEstablishedCallback onSubscriptionEstablished)
{
    ReturnErrorOnFailure(mSeekRangeStartSubscriber.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mSeekRangeStartSubscriber.SubscribeAttribute(context, successFn, failureFn, minInterval, maxInterval,
                                                        onSubscriptionEstablished);
}

/**
 * @brief Application Launcher cluster
 */
CHIP_ERROR
CastingServer::ApplicationLauncher_LaunchApp(TargetEndpointInfo * endpoint,
                                             chip::app::Clusters::ApplicationLauncher::Structs::ApplicationStruct::Type application,
                                             chip::Optional<chip::ByteSpan> data, std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mLaunchAppCommand.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mLaunchAppCommand.Invoke(application, data, responseCallback);
}

CHIP_ERROR
CastingServer::ApplicationLauncher_StopApp(TargetEndpointInfo * endpoint,
                                           chip::app::Clusters::ApplicationLauncher::Structs::ApplicationStruct::Type application,
                                           std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mStopAppCommand.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mStopAppCommand.Invoke(application, responseCallback);
}

CHIP_ERROR
CastingServer::ApplicationLauncher_HideApp(TargetEndpointInfo * endpoint,
                                           chip::app::Clusters::ApplicationLauncher::Structs::ApplicationStruct::Type application,
                                           std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mHideAppCommand.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mHideAppCommand.Invoke(application, responseCallback);
}

CHIP_ERROR CastingServer::ApplicationLauncher_SubscribeToCurrentApp(
    TargetEndpointInfo * endpoint, void * context,
    ReadResponseSuccessCallback<chip::app::Clusters::ApplicationLauncher::Attributes::CurrentApp::TypeInfo::DecodableArgType>
        successFn,
    ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
    SubscriptionEstablishedCallback onSubscriptionEstablished)
{
    ReturnErrorOnFailure(mCurrentAppSubscriber.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mCurrentAppSubscriber.SubscribeAttribute(context, successFn, failureFn, minInterval, maxInterval,
                                                    onSubscriptionEstablished);
}

/**
 * @brief Target Navigator cluster
 */
CHIP_ERROR CastingServer::TargetNavigator_NavigateTarget(TargetEndpointInfo * endpoint, const uint8_t target,
                                                         const chip::Optional<chip::CharSpan> data,
                                                         std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mNavigateTargetCommand.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mNavigateTargetCommand.Invoke(target, data, responseCallback);
}

CHIP_ERROR CastingServer::TargetNavigator_SubscribeToTargetList(
    TargetEndpointInfo * endpoint, void * context,
    ReadResponseSuccessCallback<chip::app::Clusters::TargetNavigator::Attributes::TargetList::TypeInfo::DecodableArgType> successFn,
    ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
    SubscriptionEstablishedCallback onSubscriptionEstablished)
{
    ReturnErrorOnFailure(mTargetListSubscriber.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mTargetListSubscriber.SubscribeAttribute(context, successFn, failureFn, minInterval, maxInterval,
                                                    onSubscriptionEstablished);
}

CHIP_ERROR CastingServer::TargetNavigator_SubscribeToCurrentTarget(
    TargetEndpointInfo * endpoint, void * context,
    ReadResponseSuccessCallback<chip::app::Clusters::TargetNavigator::Attributes::CurrentTarget::TypeInfo::DecodableArgType>
        successFn,
    ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
    SubscriptionEstablishedCallback onSubscriptionEstablished)
{
    ReturnErrorOnFailure(mCurrentTargetSubscriber.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mCurrentTargetSubscriber.SubscribeAttribute(context, successFn, failureFn, minInterval, maxInterval,
                                                       onSubscriptionEstablished);
}

/**
 * @brief Keypad Input cluster
 */
CHIP_ERROR CastingServer::KeypadInput_SendKey(TargetEndpointInfo * endpoint,
                                              const chip::app::Clusters::KeypadInput::CECKeyCodeEnum keyCode,
                                              std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mSendKeyCommand.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mSendKeyCommand.Invoke(keyCode, responseCallback);
}

/**
 * @brief Application Basic cluster
 */
CHIP_ERROR CastingServer::ApplicationBasic_SubscribeToVendorName(
    TargetEndpointInfo * endpoint, void * context,
    chip::Controller::ReadResponseSuccessCallback<
        chip::app::Clusters::ApplicationBasic::Attributes::VendorName::TypeInfo::DecodableArgType>
        successFn,
    chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
    chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished)
{
    ReturnErrorOnFailure(mVendorNameSubscriber.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mVendorNameSubscriber.SubscribeAttribute(context, successFn, failureFn, minInterval, maxInterval,
                                                    onSubscriptionEstablished);
}

CHIP_ERROR
CastingServer::ApplicationBasic_SubscribeToVendorID(
    TargetEndpointInfo * endpoint, void * context,
    chip::Controller::ReadResponseSuccessCallback<
        chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo::DecodableArgType>
        successFn,
    chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
    chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished)
{
    ReturnErrorOnFailure(mVendorIDSubscriber.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mVendorIDSubscriber.SubscribeAttribute(context, successFn, failureFn, minInterval, maxInterval,
                                                  onSubscriptionEstablished);
}

CHIP_ERROR CastingServer::ApplicationBasic_SubscribeToApplicationName(
    TargetEndpointInfo * endpoint, void * context,
    chip::Controller::ReadResponseSuccessCallback<
        chip::app::Clusters::ApplicationBasic::Attributes::ApplicationName::TypeInfo::DecodableArgType>
        successFn,
    chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
    chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished)
{
    ReturnErrorOnFailure(mApplicationNameSubscriber.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mApplicationNameSubscriber.SubscribeAttribute(context, successFn, failureFn, minInterval, maxInterval,
                                                         onSubscriptionEstablished);
}

CHIP_ERROR
CastingServer::ApplicationBasic_SubscribeToProductID(
    TargetEndpointInfo * endpoint, void * context,
    chip::Controller::ReadResponseSuccessCallback<
        chip::app::Clusters::ApplicationBasic::Attributes::ProductID::TypeInfo::DecodableArgType>
        successFn,
    chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
    chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished)
{
    ReturnErrorOnFailure(mProductIDSubscriber.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mProductIDSubscriber.SubscribeAttribute(context, successFn, failureFn, minInterval, maxInterval,
                                                   onSubscriptionEstablished);
}

CHIP_ERROR CastingServer::ApplicationBasic_SubscribeToApplication(
    TargetEndpointInfo * endpoint, void * context,
    chip::Controller::ReadResponseSuccessCallback<
        chip::app::Clusters::ApplicationBasic::Attributes::Application::TypeInfo::DecodableArgType>
        successFn,
    chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
    chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished)
{
    ReturnErrorOnFailure(mApplicationSubscriber.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mApplicationSubscriber.SubscribeAttribute(context, successFn, failureFn, minInterval, maxInterval,
                                                     onSubscriptionEstablished);
}

CHIP_ERROR
CastingServer::ApplicationBasic_SubscribeToStatus(
    TargetEndpointInfo * endpoint, void * context,
    chip::Controller::ReadResponseSuccessCallback<
        chip::app::Clusters::ApplicationBasic::Attributes::Status::TypeInfo::DecodableArgType>
        successFn,
    chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
    chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished)
{
    ReturnErrorOnFailure(mStatusSubscriber.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mStatusSubscriber.SubscribeAttribute(context, successFn, failureFn, minInterval, maxInterval, onSubscriptionEstablished);
}

CHIP_ERROR CastingServer::ApplicationBasic_SubscribeToApplicationVersion(
    TargetEndpointInfo * endpoint, void * context,
    chip::Controller::ReadResponseSuccessCallback<
        chip::app::Clusters::ApplicationBasic::Attributes::ApplicationVersion::TypeInfo::DecodableArgType>
        successFn,
    chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
    chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished)
{
    ReturnErrorOnFailure(mApplicationVersionSubscriber.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mApplicationVersionSubscriber.SubscribeAttribute(context, successFn, failureFn, minInterval, maxInterval,
                                                            onSubscriptionEstablished);
}

CHIP_ERROR CastingServer::ApplicationBasic_SubscribeToAllowedVendorList(
    TargetEndpointInfo * endpoint, void * context,
    chip::Controller::ReadResponseSuccessCallback<
        chip::app::Clusters::ApplicationBasic::Attributes::AllowedVendorList::TypeInfo::DecodableArgType>
        successFn,
    chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
    chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished)
{
    ReturnErrorOnFailure(mAllowedVendorListSubscriber.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mAllowedVendorListSubscriber.SubscribeAttribute(context, successFn, failureFn, minInterval, maxInterval,
                                                           onSubscriptionEstablished);
}

CHIP_ERROR CastingServer::ApplicationBasic_ReadVendorName(
    TargetEndpointInfo * endpoint, void * context,
    chip::Controller::ReadResponseSuccessCallback<
        chip::app::Clusters::ApplicationBasic::Attributes::VendorName::TypeInfo::DecodableArgType>
        successFn,
    chip::Controller::ReadResponseFailureCallback failureFn)
{
    ReturnErrorOnFailure(mVendorNameReader.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mVendorNameReader.ReadAttribute(context, successFn, failureFn);
}

CHIP_ERROR
CastingServer::ApplicationBasic_ReadVendorID(
    TargetEndpointInfo * endpoint, void * context,
    chip::Controller::ReadResponseSuccessCallback<
        chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo::DecodableArgType>
        successFn,
    chip::Controller::ReadResponseFailureCallback failureFn)
{
    ReturnErrorOnFailure(mVendorIDReader.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mVendorIDReader.ReadAttribute(context, successFn, failureFn);
}

CHIP_ERROR CastingServer::ApplicationBasic_ReadApplicationName(
    TargetEndpointInfo * endpoint, void * context,
    chip::Controller::ReadResponseSuccessCallback<
        chip::app::Clusters::ApplicationBasic::Attributes::ApplicationName::TypeInfo::DecodableArgType>
        successFn,
    chip::Controller::ReadResponseFailureCallback failureFn)
{
    ReturnErrorOnFailure(mApplicationNameReader.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mApplicationNameReader.ReadAttribute(context, successFn, failureFn);
}

CHIP_ERROR
CastingServer::ApplicationBasic_ReadProductID(
    TargetEndpointInfo * endpoint, void * context,
    chip::Controller::ReadResponseSuccessCallback<
        chip::app::Clusters::ApplicationBasic::Attributes::ProductID::TypeInfo::DecodableArgType>
        successFn,
    chip::Controller::ReadResponseFailureCallback failureFn)
{
    ReturnErrorOnFailure(mProductIDReader.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mProductIDReader.ReadAttribute(context, successFn, failureFn);
}

CHIP_ERROR CastingServer::ApplicationBasic_ReadApplication(
    TargetEndpointInfo * endpoint, void * context,
    chip::Controller::ReadResponseSuccessCallback<
        chip::app::Clusters::ApplicationBasic::Attributes::Application::TypeInfo::DecodableArgType>
        successFn,
    chip::Controller::ReadResponseFailureCallback failureFn)
{
    ReturnErrorOnFailure(mApplicationReader.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mApplicationReader.ReadAttribute(context, successFn, failureFn);
}

CHIP_ERROR
CastingServer::ApplicationBasic_ReadStatus(
    TargetEndpointInfo * endpoint, void * context,
    chip::Controller::ReadResponseSuccessCallback<
        chip::app::Clusters::ApplicationBasic::Attributes::Status::TypeInfo::DecodableArgType>
        successFn,
    chip::Controller::ReadResponseFailureCallback failureFn)
{
    ReturnErrorOnFailure(mStatusReader.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mStatusReader.ReadAttribute(context, successFn, failureFn);
}

CHIP_ERROR CastingServer::ApplicationBasic_ReadApplicationVersion(
    TargetEndpointInfo * endpoint, void * context,
    chip::Controller::ReadResponseSuccessCallback<
        chip::app::Clusters::ApplicationBasic::Attributes::ApplicationVersion::TypeInfo::DecodableArgType>
        successFn,
    chip::Controller::ReadResponseFailureCallback failureFn)
{
    ReturnErrorOnFailure(mApplicationVersionReader.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mApplicationVersionReader.ReadAttribute(context, successFn, failureFn);
}

CHIP_ERROR CastingServer::ApplicationBasic_ReadAllowedVendorList(
    TargetEndpointInfo * endpoint, void * context,
    chip::Controller::ReadResponseSuccessCallback<
        chip::app::Clusters::ApplicationBasic::Attributes::AllowedVendorList::TypeInfo::DecodableArgType>
        successFn,
    chip::Controller::ReadResponseFailureCallback failureFn)
{
    ReturnErrorOnFailure(mAllowedVendorListReader.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mAllowedVendorListReader.ReadAttribute(context, successFn, failureFn);
}

/*
 * @brief Channel cluster
 */
CHIP_ERROR CastingServer::Channel_ChangeChannelCommand(TargetEndpointInfo * endpoint, const chip::CharSpan & match,
                                                       std::function<void(CHIP_ERROR)> responseCallback)
{
    ReturnErrorOnFailure(mChangeChannelCommand.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mChangeChannelCommand.Invoke(match, responseCallback);
}

CHIP_ERROR CastingServer::Channel_SubscribeToLineup(
    TargetEndpointInfo * endpoint, void * context,
    chip::Controller::ReadResponseSuccessCallback<chip::app::Clusters::Channel::Attributes::Lineup::TypeInfo::DecodableArgType>
        successFn,
    chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval, uint16_t maxInterval,
    chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished)
{
    ReturnErrorOnFailure(mLineupSubscriber.SetTarget(mActiveTargetVideoPlayerInfo, endpoint->GetEndpointId()));
    return mLineupSubscriber.SubscribeAttribute(context, successFn, failureFn, minInterval, maxInterval, onSubscriptionEstablished);
}
