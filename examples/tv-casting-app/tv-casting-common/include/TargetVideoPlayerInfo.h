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

#include <platform/CHIPDeviceLayer.h>
#include "app/clusters/bindings/BindingManager.h"
#include "TargetEndpointInfo.h"

using namespace chip;

class TargetVideoPlayerInfo
{
public:
    TargetVideoPlayerInfo() :
        mOnConnectedCallback(HandleDeviceConnected, this), mOnConnectionFailureCallback(HandleDeviceConnectionFailure, this)
    {}

    bool IsInitialized() { return mInitialized; }
    NodeId GetNodeId() const { return mNodeId; }
    FabricIndex GetFabricIndex() const { return mFabricIndex; }
    OperationalDeviceProxy * GetOperationalDeviceProxy() const { return mOperationalDeviceProxy; }

    CHIP_ERROR Initialize(NodeId nodeId, FabricIndex fabricIndex);
    TargetEndpointInfo * GetOrAddEndpoint(EndpointId endpointId);
    TargetEndpointInfo * GetEndpoint(EndpointId endpointId);
    bool HasEndpoint(EndpointId endpointId);
    void PrintInfo();

private:
    static void HandleDeviceConnected(void * context, OperationalDeviceProxy * device)
    {
        TargetVideoPlayerInfo * _this  = static_cast<TargetVideoPlayerInfo *>(context);
        _this->mOperationalDeviceProxy = device;
        _this->mInitialized            = true;
        ChipLogProgress(AppServer, "HandleDeviceConnected created an instance of OperationalDeviceProxy");
    }

    static void HandleDeviceConnectionFailure(void * context, PeerId peerId, CHIP_ERROR error)
    {
        TargetVideoPlayerInfo * _this  = static_cast<TargetVideoPlayerInfo *>(context);
        _this->mOperationalDeviceProxy = nullptr;
    }

    static constexpr size_t kMaxNumberOfEndpoints = 5;
    TargetEndpointInfo mEndpoints[kMaxNumberOfEndpoints];
    NodeId mNodeId;
    FabricIndex mFabricIndex;
    OperationalDeviceProxy * mOperationalDeviceProxy;

    Callback::Callback<OnDeviceConnected> mOnConnectedCallback;
    Callback::Callback<OnDeviceConnectionFailure> mOnConnectionFailureCallback;

    bool mInitialized = false;
};