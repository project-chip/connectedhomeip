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
#include "app/clusters/bindings/BindingManager.h"
#include <platform/CHIPDeviceLayer.h>

constexpr size_t kMaxNumberOfEndpoints = 5;

class TargetVideoPlayerInfo
{
public:
    TargetVideoPlayerInfo() :
        mOnConnectedCallback(HandleDeviceConnected, this), mOnConnectionFailureCallback(HandleDeviceConnectionFailure, this)
    {}

    bool operator==(const TargetVideoPlayerInfo & other) { return this->mNodeId == other.mNodeId; }

    bool IsInitialized() { return mInitialized; }
    uint16_t GetVendorId() const { return mVendorId; }
    uint16_t GetProductId() const { return mProductId; }
    uint16_t GetDeviceType() const { return mDeviceType; }
    chip::NodeId GetNodeId() const { return mNodeId; }
    chip::FabricIndex GetFabricIndex() const { return mFabricIndex; }
    const char * GetDeviceName() const { return mDeviceName; }
    size_t GetNumIPs() const { return mNumIPs; }
    const chip::Inet::IPAddress * GetIpAddresses() const { return mIpAddress; }
    bool IsSameAs(const chip::Dnssd::DiscoveredNodeData * discoveredNodeData);
    bool IsSameAs(const char * deviceName, size_t numIPs, const chip::Inet::IPAddress * ipAddresses);

    chip::OperationalDeviceProxy * GetOperationalDeviceProxy()
    {
        if (mDeviceProxy.ConnectionReady())
        {
            return &mDeviceProxy;
        }
        return nullptr;
    }

    CHIP_ERROR Initialize(chip::NodeId nodeId, chip::FabricIndex fabricIndex,
                          std::function<void(TargetVideoPlayerInfo *)> onConnectionSuccess,
                          std::function<void(CHIP_ERROR)> onConnectionFailure, uint16_t vendorId = 0, uint16_t productId = 0,
                          uint16_t deviceType = 0, const char * deviceName = {}, size_t numIPs = 0,
                          chip::Inet::IPAddress * ipAddressList = nullptr);
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
        TargetVideoPlayerInfo * _this = static_cast<TargetVideoPlayerInfo *>(context);
        _this->mDeviceProxy           = chip::OperationalDeviceProxy(&exchangeMgr, sessionHandle);
        _this->mInitialized           = true;
        ChipLogProgress(AppServer,
                        "HandleDeviceConnected created an instance of OperationalDeviceProxy for nodeId: 0x" ChipLogFormatX64
                        ", fabricIndex: %d",
                        ChipLogValueX64(_this->GetNodeId()), _this->GetFabricIndex());

        if (_this->mOnConnectionSuccessClientCallback)
        {
            ChipLogProgress(AppServer, "HandleDeviceConnected calling mOnConnectionSuccessClientCallback");
            _this->mOnConnectionSuccessClientCallback(_this);
        }
    }

    static void HandleDeviceConnectionFailure(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error)
    {
        ChipLogError(AppServer,
                     "HandleDeviceConnectionFailure called for peerId.nodeId: 0x" ChipLogFormatX64
                     ", peer.fabricIndex: %d with error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(peerId.GetNodeId()), peerId.GetFabricIndex(), error.Format());
        TargetVideoPlayerInfo * _this = static_cast<TargetVideoPlayerInfo *>(context);
        _this->mDeviceProxy           = chip::OperationalDeviceProxy();
        if (_this->mOnConnectionFailureClientCallback)
        {
            ChipLogProgress(AppServer, "HandleDeviceConnectionFailure calling mOnConnectionFailureClientCallback");
            _this->mOnConnectionFailureClientCallback(error);
        }
    }

    TargetEndpointInfo mEndpoints[kMaxNumberOfEndpoints];
    chip::NodeId mNodeId;
    chip::FabricIndex mFabricIndex;
    chip::OperationalDeviceProxy mDeviceProxy;
    uint16_t mVendorId                                   = 0;
    uint16_t mProductId                                  = 0;
    uint16_t mDeviceType                                 = 0;
    char mDeviceName[chip::Dnssd::kMaxDeviceNameLen + 1] = {};
    size_t mNumIPs                                       = 0; // number of valid IP addresses
    chip::Inet::IPAddress mIpAddress[chip::Dnssd::CommonResolutionData::kMaxIPAddresses];

    chip::Callback::Callback<chip::OnDeviceConnected> mOnConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnConnectionFailureCallback;
    std::function<void(TargetVideoPlayerInfo *)> mOnConnectionSuccessClientCallback;
    std::function<void(CHIP_ERROR)> mOnConnectionFailureClientCallback;

    bool mInitialized = false;
};
