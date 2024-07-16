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

#include <app/server/Server.h>

using namespace chip;

CASEClientPool<CHIP_CONFIG_DEVICE_MAX_ACTIVE_CASE_CLIENTS> gCASEClientPool;

CHIP_ERROR TargetVideoPlayerInfo::Initialize(NodeId nodeId, FabricIndex fabricIndex,
                                             std::function<void(TargetVideoPlayerInfo *)> onConnectionSuccess,
                                             std::function<void(CHIP_ERROR)> onConnectionFailure, uint16_t vendorId,
                                             uint16_t productId, chip::DeviceTypeId deviceType, const char * deviceName,
                                             const char * hostName, size_t numIPs, chip::Inet::IPAddress * ipAddress, uint16_t port,
                                             const char * instanceName, chip::System::Clock::Timestamp lastDiscovered)
{
    ChipLogProgress(NotSpecified, "TargetVideoPlayerInfo nodeId=0x" ChipLogFormatX64 " fabricIndex=%d", ChipLogValueX64(nodeId),
                    fabricIndex);
    mNodeId         = nodeId;
    mFabricIndex    = fabricIndex;
    mVendorId       = vendorId;
    mProductId      = productId;
    mDeviceType     = deviceType;
    mNumIPs         = numIPs;
    mPort           = port;
    mLastDiscovered = lastDiscovered;
    for (size_t i = 0; i < numIPs && i < chip::Dnssd::CommonResolutionData::kMaxIPAddresses; i++)
    {
        mIpAddress[i] = ipAddress[i];
    }

    memset(mDeviceName, '\0', sizeof(mDeviceName));
    if (deviceName != nullptr)
    {
        chip::Platform::CopyString(mDeviceName, chip::Dnssd::kMaxDeviceNameLen, deviceName);
    }

    memset(mHostName, '\0', sizeof(mHostName));
    if (hostName != nullptr)
    {
        chip::Platform::CopyString(mHostName, chip::Dnssd::kHostNameMaxLength, hostName);
    }

    memset(mInstanceName, '\0', sizeof(mInstanceName));
    if (instanceName != nullptr)
    {
        chip::Platform::CopyString(mInstanceName, chip::Dnssd::Commission::kInstanceNameMaxLength + 1, instanceName);
    }

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

void TargetVideoPlayerInfo::Reset()
{
    ChipLogProgress(NotSpecified, "TargetVideoPlayerInfo Reset() called");
    mInitialized    = false;
    mNodeId         = 0;
    mFabricIndex    = 0;
    mVendorId       = 0;
    mProductId      = 0;
    mDeviceType     = 0;
    mLastDiscovered = chip::System::Clock::kZero;
    memset(mDeviceName, '\0', sizeof(mDeviceName));
    memset(mHostName, '\0', sizeof(mHostName));
    mDeviceProxy = nullptr;
    for (auto & endpointInfo : mEndpoints)
    {
        endpointInfo.Reset();
    }
    for (size_t i = 0; i < mNumIPs && i < chip::Dnssd::CommonResolutionData::kMaxIPAddresses; i++)
    {
        mIpAddress[i] = chip::Inet::IPAddress();
    }
    mNumIPs = 0;
}

CHIP_ERROR TargetVideoPlayerInfo::FindOrEstablishCASESession(std::function<void(TargetVideoPlayerInfo *)> onConnectionSuccess,
                                                             std::function<void(CHIP_ERROR)> onConnectionFailure)
{
    ChipLogProgress(AppServer, "TargetVideoPlayerInfo::FindOrEstablishCASESession called");

    VideoPlayerConnectionContext * connectionContext = new VideoPlayerConnectionContext(
        this, HandleDeviceConnected, HandleDeviceConnectionFailure, onConnectionSuccess, onConnectionFailure);
    Server * server = &(chip::Server::GetInstance());
    server->GetCASESessionManager()->FindOrEstablishSession(ScopedNodeId(mNodeId, mFabricIndex),
                                                            connectionContext->mOnConnectedCallback,
                                                            connectionContext->mOnConnectionFailureCallback);
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
    ChipLogProgress(NotSpecified, " TargetVideoPlayerInfo deviceName=%s nodeId=0x" ChipLogFormatX64 " fabric index=%d", mDeviceName,
                    ChipLogValueX64(mNodeId), mFabricIndex);
    if (mMACAddress.size() > 0)
    {
        ChipLogProgress(NotSpecified, "  MACAddress=%.*s", static_cast<int>(mMACAddress.size()), mMACAddress.data());
    }

    for (auto & endpointInfo : mEndpoints)
    {
        if (endpointInfo.IsInitialized())
        {
            endpointInfo.PrintInfo();
        }
    }
}

bool TargetVideoPlayerInfo::IsSameAs(const char * hostName, const char * deviceName, size_t numIPs,
                                     const chip::Inet::IPAddress * ipAddresses)
{
    // return true if the hostNames match
    if (strcmp(mHostName, hostName) == 0)
    {
        return true;
    }

    // return false because deviceNames are different
    if (strcmp(mDeviceName, deviceName) != 0)
    {
        return false;
    }

    // return false because not even a single IP Address matches
    if (mNumIPs > 0)
    {
        bool matchFound = false;
        for (size_t i = 0; i < mNumIPs && i < chip::Dnssd::CommonResolutionData::kMaxIPAddresses; i++)
        {
            for (size_t j = 0; j < numIPs && j < chip::Dnssd::CommonResolutionData::kMaxIPAddresses; j++)
            {
                if (mIpAddress[i] == ipAddresses[j])
                {
                    matchFound = true;
                    break;
                }
            }
            if (matchFound)
            {
                break;
            }
        }

        if (!matchFound)
        {
            return false;
        }
    }

    return true;
}

bool TargetVideoPlayerInfo::IsSameAs(const chip::Dnssd::CommissionNodeData * discoveredNodeData)
{
    // return false because 'this' VideoPlayer is not null
    if (discoveredNodeData == nullptr)
    {
        return false;
    }

    return IsSameAs(discoveredNodeData->hostName, discoveredNodeData->deviceName, discoveredNodeData->numIPs,
                    discoveredNodeData->ipAddress);
}
