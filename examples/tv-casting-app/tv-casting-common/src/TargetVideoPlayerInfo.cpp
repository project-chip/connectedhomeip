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
#include "TargetVideoPlayerInfo.h"

using namespace chip;

CASEClientPool<CHIP_CONFIG_DEVICE_MAX_ACTIVE_CASE_CLIENTS> gCASEClientPool;

CHIP_ERROR TargetVideoPlayerInfo::Initialize(NodeId nodeId, FabricIndex fabricIndex,
                                             std::function<void(TargetVideoPlayerInfo *)> onConnectionSuccess,
                                             std::function<void(CHIP_ERROR)> onConnectionFailure, uint16_t vendorId,
                                             uint16_t productId, uint16_t deviceType, const char * deviceName)
{
    ChipLogProgress(NotSpecified, "TargetVideoPlayerInfo nodeId=0x" ChipLogFormatX64 " fabricIndex=%d", ChipLogValueX64(nodeId),
                    fabricIndex);
    mNodeId      = nodeId;
    mFabricIndex = fabricIndex;
    mVendorId    = vendorId;
    mProductId   = productId;
    mDeviceType  = deviceType;
    chip::Platform::CopyString(mDeviceName, chip::Dnssd::kMaxDeviceNameLen + 1, deviceName);
    for (auto & endpointInfo : mEndpoints)
    {
        endpointInfo.Reset();
    }

    if (onConnectionSuccess && onConnectionFailure)
    {
        ReturnErrorOnFailure(FindOrEstablishCASESession(onConnectionSuccess, onConnectionFailure));
    }

    mInitialized = true;
    return CHIP_NO_ERROR;
}

CHIP_ERROR TargetVideoPlayerInfo::FindOrEstablishCASESession(std::function<void(TargetVideoPlayerInfo *)> onConnectionSuccess,
                                                             std::function<void(CHIP_ERROR)> onConnectionFailure)
{
    mOnConnectionSuccessClientCallback = onConnectionSuccess;
    mOnConnectionFailureClientCallback = onConnectionFailure;
    Server * server                    = &(chip::Server::GetInstance());
    server->GetCASESessionManager()->FindOrEstablishSession(ScopedNodeId(mNodeId, mFabricIndex), &mOnConnectedCallback,
                                                            &mOnConnectionFailureCallback);
    return CHIP_NO_ERROR;
}

TargetEndpointInfo * TargetVideoPlayerInfo::GetOrAddEndpoint(EndpointId endpointId)
{
    TargetEndpointInfo * endpoint = GetEndpoint(endpointId);
    if (endpoint != nullptr)
    {
        return endpoint;
    }
    for (auto & endpointInfo : mEndpoints)
    {
        if (!endpointInfo.IsInitialized())
        {
            endpointInfo.Initialize(endpointId);
            return &endpointInfo;
        }
    }
    return nullptr;
}

TargetEndpointInfo * TargetVideoPlayerInfo::GetEndpoint(EndpointId endpointId)
{
    for (auto & endpointInfo : mEndpoints)
    {
        if (endpointInfo.IsInitialized() && endpointInfo.GetEndpointId() == endpointId)
        {
            return &endpointInfo;
        }
    }
    return nullptr;
}

TargetEndpointInfo * TargetVideoPlayerInfo::GetEndpoints()
{
    return mEndpoints;
}

bool TargetVideoPlayerInfo::HasEndpoint(EndpointId endpointId)
{
    for (auto & endpointInfo : mEndpoints)
    {
        if (endpointInfo.IsInitialized() && endpointInfo.GetEndpointId() == endpointId)
        {
            return true;
        }
    }
    return false;
}

void TargetVideoPlayerInfo::PrintInfo()
{
    ChipLogProgress(NotSpecified, " TargetVideoPlayerInfo nodeId=0x" ChipLogFormatX64 " fabric index=%d", ChipLogValueX64(mNodeId),
                    mFabricIndex);
    for (auto & endpointInfo : mEndpoints)
    {
        if (endpointInfo.IsInitialized())
        {
            endpointInfo.PrintInfo();
        }
    }
}
