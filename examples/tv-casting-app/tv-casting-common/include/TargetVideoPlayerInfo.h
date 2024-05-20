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

#include "TargetEndpointInfo.h"

#include <app/OperationalSessionSetup.h>
#include <platform/CHIPDeviceLayer.h>

#include <app-common/zap-generated/cluster-objects.h>

#include <string.h>
#include <system/SystemClock.h>
#include <system/SystemLayer.h>

inline constexpr size_t kMaxNumberOfEndpoints = 5;

class TargetVideoPlayerInfo;
class VideoPlayerConnectionContext
{
public:
    VideoPlayerConnectionContext(TargetVideoPlayerInfo * targetVideoPlayerInfo, chip::OnDeviceConnected handleDeviceConnected,
                                 chip::OnDeviceConnectionFailure handleConnectionFailure,
                                 std::function<void(TargetVideoPlayerInfo *)> onConnectionSuccess,
                                 std::function<void(CHIP_ERROR)> onConnectionFailure)
    {
        mTargetVideoPlayerInfo       = targetVideoPlayerInfo;
        mOnConnectedCallback         = new chip::Callback::Callback<chip::OnDeviceConnected>(handleDeviceConnected, this);
        mOnConnectionFailureCallback = new chip::Callback::Callback<chip::OnDeviceConnectionFailure>(handleConnectionFailure, this);
        mOnConnectionSuccessClientCallback = onConnectionSuccess;
        mOnConnectionFailureClientCallback = onConnectionFailure;
    }

    ~VideoPlayerConnectionContext()
    {
        if (mOnConnectedCallback != nullptr)
        {
            delete mOnConnectedCallback;
        }

        if (mOnConnectionFailureCallback != nullptr)
        {
            delete mOnConnectionFailureCallback;
        }
    }

    TargetVideoPlayerInfo * mTargetVideoPlayerInfo;
    chip::Callback::Callback<chip::OnDeviceConnected> * mOnConnectedCallback                 = nullptr;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> * mOnConnectionFailureCallback = nullptr;
    std::function<void(TargetVideoPlayerInfo *)> mOnConnectionSuccessClientCallback          = {};
    std::function<void(CHIP_ERROR)> mOnConnectionFailureClientCallback                       = {};
};

class TargetVideoPlayerInfo
{
public:
    TargetVideoPlayerInfo() {}

    bool operator==(const TargetVideoPlayerInfo & other) const { return this->mNodeId == other.mNodeId; }

    bool IsInitialized() { return mInitialized; }
    void Reset();
    uint16_t GetVendorId() const { return mVendorId; }
    uint16_t GetProductId() const { return mProductId; }
    chip::DeviceTypeId GetDeviceType() const { return mDeviceType; }
    chip::NodeId GetNodeId() const { return mNodeId; }
    chip::FabricIndex GetFabricIndex() const { return mFabricIndex; }
    const char * GetDeviceName() const { return mDeviceName; }
    const char * GetHostName() const { return mHostName; }
    size_t GetNumIPs() const { return mNumIPs; }
    const chip::Inet::IPAddress * GetIpAddresses() const { return mIpAddress; }
    bool IsSameAs(const chip::Dnssd::CommissionNodeData * discoveredNodeData);
    bool IsSameAs(const char * hostName, const char * deviceName, size_t numIPs, const chip::Inet::IPAddress * ipAddresses);

    uint16_t GetPort() const { return mPort; }
    const char * GetInstanceName() const { return mInstanceName; }
    chip::CharSpan * GetMACAddress() { return &mMACAddress; }
    void SetIsAsleep(bool isAsleep) { mIsAsleep = isAsleep; }
    bool IsAsleep() { return mIsAsleep; }
    void SetMACAddress(chip::CharSpan MACAddress)
    {
        memcpy(mMACAddressBuf, MACAddress.data(), sizeof(mMACAddressBuf));
        mMACAddress = chip::CharSpan(mMACAddressBuf, sizeof(mMACAddressBuf));
    }
    chip::System::Clock::Timestamp GetLastDiscovered() { return mLastDiscovered; }
    void SetLastDiscovered(chip::System::Clock::Timestamp lastDiscovered) { mLastDiscovered = lastDiscovered; }
    bool WasRecentlyDiscoverable()
    {
#ifdef CHIP_DEVICE_CONFIG_STR_CACHE_LAST_DISCOVERED_HOURS
        // it was recently discoverable if its mLastDiscovered.count is within
        // CHIP_DEVICE_CONFIG_STR_CACHE_LAST_DISCOVERED_HOURS of current time
        chip::System::Clock::Timestamp currentUnixTimeMS = chip::System::Clock::kZero;
        VerifyOrReturnValue(chip::System::SystemClock().GetClock_RealTimeMS(currentUnixTimeMS) == CHIP_NO_ERROR, true);
        ChipLogProgress(AppServer, "WasRecentlyDiscoverable currentUnixTimeMS: %lu mLastDiscovered: %lu",
                        static_cast<unsigned long>(currentUnixTimeMS.count()), static_cast<unsigned long>(mLastDiscovered.count()));
        return mLastDiscovered.count() >
            currentUnixTimeMS.count() - CHIP_DEVICE_CONFIG_STR_CACHE_LAST_DISCOVERED_HOURS * 60 * 60 * 1000;
#else
        return true;
#endif // CHIP_DEVICE_CONFIG_STR_CACHE_LAST_DISCOVERED_HOURS
    }

    chip::OperationalDeviceProxy * GetOperationalDeviceProxy()
    {
        if (mDeviceProxy != nullptr && mDeviceProxy->ConnectionReady())
        {
            return mDeviceProxy;
        }
        return nullptr;
    }

    CHIP_ERROR Initialize(chip::NodeId nodeId, chip::FabricIndex fabricIndex,
                          std::function<void(TargetVideoPlayerInfo *)> onConnectionSuccess,
                          std::function<void(CHIP_ERROR)> onConnectionFailure, uint16_t vendorId = 0, uint16_t productId = 0,
                          chip::DeviceTypeId deviceType = 0, const char * deviceName = {}, const char * hostName = {},
                          size_t numIPs = 0, chip::Inet::IPAddress * ipAddressList = nullptr, uint16_t port = 0,
                          const char * instanceName                     = {},
                          chip::System::Clock::Timestamp lastDiscovered = chip::System::Clock::kZero);
    CHIP_ERROR FindOrEstablishCASESession(std::function<void(TargetVideoPlayerInfo *)> onConnectionSuccess,
                                          std::function<void(CHIP_ERROR)> onConnectionFailure);
    TargetEndpointInfo * GetOrAddEndpoint(chip::EndpointId endpointId);
    TargetEndpointInfo * GetEndpoint(chip::EndpointId endpointId);
    TargetEndpointInfo * GetEndpoints();
    bool HasEndpoint(chip::EndpointId endpointId);
    void PrintInfo();

private:
    static void HandleDeviceConnected(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                      const chip::SessionHandle & sessionHandle)
    {
        VideoPlayerConnectionContext * connectionContext = static_cast<VideoPlayerConnectionContext *>(context);
        if (connectionContext == nullptr || connectionContext->mTargetVideoPlayerInfo == nullptr)
        {
            ChipLogError(AppServer, "HandleDeviceConnected called with null context or null context.targetVideoPlayerInfo");
            return;
        }
        if (connectionContext->mTargetVideoPlayerInfo->mDeviceProxy != nullptr)
        {
            ChipLogProgress(AppServer, "HandleDeviceConnected deleting mDeviceProxy");
            delete connectionContext->mTargetVideoPlayerInfo->mDeviceProxy;
            ChipLogProgress(AppServer, "HandleDeviceConnected deleted mDeviceProxy");
        }
        connectionContext->mTargetVideoPlayerInfo->mDeviceProxy = new chip::OperationalDeviceProxy(&exchangeMgr, sessionHandle);
        connectionContext->mTargetVideoPlayerInfo->mInitialized = true;
        ChipLogProgress(AppServer,
                        "HandleDeviceConnected created an instance of OperationalDeviceProxy for nodeId: 0x" ChipLogFormatX64
                        ", fabricIndex: %d",
                        ChipLogValueX64(connectionContext->mTargetVideoPlayerInfo->GetNodeId()),
                        connectionContext->mTargetVideoPlayerInfo->GetFabricIndex());

        if (connectionContext->mOnConnectionSuccessClientCallback)
        {
            ChipLogProgress(AppServer, "HandleDeviceConnected calling mOnConnectionSuccessClientCallback");
            connectionContext->mOnConnectionSuccessClientCallback(connectionContext->mTargetVideoPlayerInfo);
        }
        delete connectionContext;
    }

    static void HandleDeviceConnectionFailure(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error)
    {
        ChipLogError(AppServer,
                     "HandleDeviceConnectionFailure called for peerId.nodeId: 0x" ChipLogFormatX64
                     ", peer.fabricIndex: %d with error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(peerId.GetNodeId()), peerId.GetFabricIndex(), error.Format());
        VideoPlayerConnectionContext * connectionContext = static_cast<VideoPlayerConnectionContext *>(context);
        if (connectionContext == nullptr || connectionContext->mTargetVideoPlayerInfo == nullptr)
        {
            ChipLogError(AppServer, "HandleDeviceConnectionFailure called with null context");
            return;
        }
        if (connectionContext->mTargetVideoPlayerInfo->mDeviceProxy != nullptr)
        {
            delete connectionContext->mTargetVideoPlayerInfo->mDeviceProxy;
        }
        connectionContext->mTargetVideoPlayerInfo->mDeviceProxy = new chip::OperationalDeviceProxy();
        if (connectionContext->mOnConnectionFailureClientCallback)
        {
            ChipLogProgress(AppServer, "HandleDeviceConnectionFailure calling mOnConnectionFailureClientCallback");
            connectionContext->mOnConnectionFailureClientCallback(error);
        }
        delete connectionContext;
    }

    TargetEndpointInfo mEndpoints[kMaxNumberOfEndpoints];
    chip::NodeId mNodeId;
    chip::FabricIndex mFabricIndex;
    chip::OperationalDeviceProxy * mDeviceProxy          = nullptr;
    uint16_t mVendorId                                   = 0;
    uint16_t mProductId                                  = 0;
    chip::DeviceTypeId mDeviceType                       = 0;
    char mDeviceName[chip::Dnssd::kMaxDeviceNameLen + 1] = {};
    char mHostName[chip::Dnssd::kHostNameMaxLength + 1]  = {};
    size_t mNumIPs                                       = 0; // number of valid IP addresses
    chip::Inet::IPAddress mIpAddress[chip::Dnssd::CommonResolutionData::kMaxIPAddresses];
    char mInstanceName[chip::Dnssd::Commission::kInstanceNameMaxLength + 1];
    uint16_t mPort;
    chip::CharSpan mMACAddress;
    char mMACAddressBuf[2 * chip::DeviceLayer::ConfigurationManager::kPrimaryMACAddressLength];
    chip::System::Clock::Timestamp mLastDiscovered;
    bool mIsAsleep    = false;
    bool mInitialized = false;
};
