/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/ClusterStateCache.h>
#include <app/OperationalSessionSetup.h>
#include <lib/core/CHIPCallback.h>

namespace chip {
namespace Controller {

enum UnpairDeviceStage : uint8_t
{
    kUnpairError,
    kReadCurrentFabricIndex,
    kSendRemoveFabric,
    kUnpairCleanUp,
};

class DLL_EXPORT DeviceUnpair : public app::ClusterStateCache::Callback
{
public:
    class Callback
    {
    public:
        virtual ~Callback(){};
        virtual void OnDeviceUnpair(NodeId remoteDeviceId, CHIP_ERROR err){};
    };

    DeviceUnpair();
    ~DeviceUnpair() {}

    void RegisterCallback(Callback * callback);
    void UnpairDevice(DeviceProxy * proxy, NodeId remoteDeviceId);

private:
    Callback * mCallback = nullptr;

    chip::Callback::Callback<OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;

    Platform::UniquePtr<app::ClusterStateCache> mAttributeCache;
    Platform::UniquePtr<app::ReadClient> mReadClient;

    DeviceProxy * mProxy = nullptr;
    Platform::SharedPtr<OperationalDeviceProxy> mOperationalDeviceProxy;
    NodeId mRemoteDeviceId;
    FabricIndex mFabricIndex = kUndefinedFabricIndex;
    UnpairDeviceStage mStage = UnpairDeviceStage::kUnpairError;

    void PerformDeviceUnpairStep(DeviceProxy * proxy, UnpairDeviceStage stage, CHIP_ERROR err);

    void ReadCurrentFabricIndex(DeviceProxy * proxy);
    void SendRemoveFabricIndex(DeviceProxy * proxy, FabricIndex fabricIndex);
    void FinishUnpairDevice(CHIP_ERROR err);

    void OnDone(app::ReadClient * apReadClient);

    static void OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr, SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailureFn(void * context, const ScopedNodeId & peerId, CHIP_ERROR error);

    static void OnRemoveFabric(void * context,
                               const chip::app::Clusters::OperationalCredentials::Commands::NOCResponse::DecodableType & data);

    static void OnCommandFailure(void * context, CHIP_ERROR error);

public:
    chip::Callback::Callback<OnDeviceConnected> & GetConnectedCallback() { return mOnDeviceConnectedCallback; }
    chip::Callback::Callback<OnDeviceConnectionFailure> & GetConnectionFailureCallback()
    {
        return mOnDeviceConnectionFailureCallback;
    }
};

} // namespace Controller
} // namespace chip
