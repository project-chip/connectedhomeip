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

CastingServer * CastingServer::castingServer_ = nullptr;
;

CastingServer * CastingServer::GetInstance()
{
    if (castingServer_ == nullptr)
    {
        castingServer_ = new CastingServer();
    }
    return castingServer_;
}

void CastingServer::InitServer()
{
    if (mInited)
    {
        return;
    }
    // DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init("/tmp/chip_tv_casting_kvs");
    DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init(CHIP_CONFIG_KVS_PATH);

    // Enter commissioning mode, open commissioning window
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    chip::Server::GetInstance().Init(initParams);

    // Initialize binding handlers
    ReturnOnFailure(InitBindingHandlers());

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
    InitServer();
    return mTargetVideoPlayerInfo.Initialize(nodeId, fabricIndex);
}

CHIP_ERROR CastingServer::DiscoverCommissioners()
{
    // Send discover commissioners request
    return mCommissionableNodeController.DiscoverCommissioners(Dnssd::DiscoveryFilter());
}

CHIP_ERROR CastingServer::OpenBasicCommissioningWindow()
{
    Server::GetInstance().GetFabricTable().DeleteAllFabrics();
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
    OperationalDeviceProxy * operationalDeviceProxy = mTargetVideoPlayerInfo.GetOperationalDeviceProxy();
    if (operationalDeviceProxy == nullptr)
    {
        ChipLogError(AppServer, "Failed in getting an instance of OperationalDeviceProxy");
        return;
    }

    chip::Controller::DescriptorCluster cluster;
    CHIP_ERROR err = cluster.Associate(operationalDeviceProxy, endpointId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Associate() failed: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

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

CHIP_ERROR CastingServer::ContentLauncherLaunchURL(const char * contentUrl, const char * contentDisplayStr)
{
    OperationalDeviceProxy * operationalDeviceProxy = mTargetVideoPlayerInfo.GetOperationalDeviceProxy();
    if (operationalDeviceProxy == nullptr)
    {
        ChipLogError(AppServer, "Failed in getting an instance of OperationalDeviceProxy");
        return CHIP_ERROR_PEER_NODE_NOT_FOUND;
    }

    ContentLauncherCluster cluster;
    CHIP_ERROR err = cluster.Associate(operationalDeviceProxy, kTvEndpoint);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Associate() failed: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }
    LaunchURL::Type request;
    request.contentURL          = chip::CharSpan::fromCharString(contentUrl);
    request.displayString       = Optional<CharSpan>(chip::CharSpan::fromCharString(contentDisplayStr));
    request.brandingInformation = MakeOptional(chip::app::Clusters::ContentLauncher::Structs::BrandingInformation::Type());
    cluster.InvokeCommand(request, nullptr, CastingServer::OnContentLauncherSuccessResponse,
                          CastingServer::OnContentLauncherFailureResponse);
    return CHIP_NO_ERROR;
}

void CastingServer::OnContentLauncherSuccessResponse(void * context, const LaunchResponse::DecodableType & response)
{
    ChipLogProgress(AppServer, "ContentLauncher: Default Success Response");
}

void CastingServer::OnContentLauncherFailureResponse(void * context, CHIP_ERROR error)
{
    ChipLogError(AppServer, "ContentLauncher: Default Failure Response: %" CHIP_ERROR_FORMAT, error.Format());
}
