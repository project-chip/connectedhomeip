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

class TargetVideoPlayerInfo
{
public:
    TargetVideoPlayerInfo() :
        mOnConnectedCallback(HandleDeviceConnected, this), mOnConnectionFailureCallback(HandleDeviceConnectionFailure, this)
    {}

    bool IsInitialized() { return mInitialized; }
    chip::NodeId GetNodeId() const { return mNodeId; }
    chip::FabricIndex GetFabricIndex() const { return mFabricIndex; }
    const chip::OperationalDeviceProxy * GetOperationalDeviceProxy() const
    {
        if (mDeviceProxy.ConnectionReady())
        {
            return &mDeviceProxy;
        }
        return nullptr;
    }

    CHIP_ERROR Initialize(chip::NodeId nodeId, chip::FabricIndex fabricIndex);
    TargetEndpointInfo * GetOrAddEndpoint(chip::EndpointId endpointId);
    TargetEndpointInfo * GetEndpoint(chip::EndpointId endpointId);
    bool HasEndpoint(chip::EndpointId endpointId);
    void PrintInfo();

private:
    static void HandleDeviceConnected(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                      chip::SessionHandle & sessionHandle)
    {
        TargetVideoPlayerInfo * _this = static_cast<TargetVideoPlayerInfo *>(context);
        _this->mDeviceProxy           = chip::OperationalDeviceProxy(&exchangeMgr, sessionHandle);
        _this->mInitialized           = true;
        ChipLogProgress(AppServer, "HandleDeviceConnected created an instance of OperationalDeviceProxy");
    }

    static void HandleDeviceConnectionFailure(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error)
    {
        TargetVideoPlayerInfo * _this = static_cast<TargetVideoPlayerInfo *>(context);
        _this->mDeviceProxy           = chip::OperationalDeviceProxy();
    }

    static constexpr size_t kMaxNumberOfEndpoints = 5;
    TargetEndpointInfo mEndpoints[kMaxNumberOfEndpoints];
    chip::NodeId mNodeId;
    chip::FabricIndex mFabricIndex;
    chip::OperationalDeviceProxy mDeviceProxy;

    chip::Callback::Callback<chip::OnDeviceConnected> mOnConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnConnectionFailureCallback;

    bool mInitialized = false;
};
