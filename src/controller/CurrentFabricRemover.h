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

#include <app/OperationalSessionSetup.h>
#include <controller/CHIPDeviceController.h>
#include <lib/core/CHIPCallback.h>

namespace chip {
namespace Controller {

typedef void (*OnCurrentFabricRemove)(void * context, NodeId remoteNodeId, CHIP_ERROR status);

/**
 * A helper class to remove fabric given some parameters.
 */
class CurrentFabricRemover
{
public:
    CurrentFabricRemover(DeviceController * controller) :
        mController(controller), mOnDeviceConnectedCallback(&OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(&OnDeviceConnectionFailureFn, this)
    {}

    enum class Step : uint8_t
    {
        // Ready to start removing a fabric.
        kAcceptRemoveFabricStart = 0,
        // Need to get Current Fabric Index.
        kReadCurrentFabricIndex,
        // Need to send Remove Fabric Command.
        kSendRemoveFabric,
    };

    /*
     * @brief
     *   Try to look up the device attached to our controller with the given
     *   remote node id and ask it to remove Fabric.
     *   If function returns an error, callback will never be be executed. Otherwise, callback will always be executed.
     *
     * @param[in] remoteNodeId The remote device Id
     * @param[in] callback The callback to call once the remote fabric is completed or not.
     */
    CHIP_ERROR RemoveCurrentFabric(NodeId remoteNodeId, Callback::Callback<OnCurrentFabricRemove> * callback);

private:
    DeviceController * mController;

    chip::Callback::Callback<OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
    chip::Callback::Callback<OnCurrentFabricRemove> * mCurrentFabricRemoveCallback;

    NodeId mRemoteNodeId;
    FabricIndex mFabricIndex = kUndefinedFabricIndex;
    Step mNextStep           = Step::kAcceptRemoveFabricStart;

    CHIP_ERROR ReadCurrentFabricIndex(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    CHIP_ERROR SendRemoveFabricIndex(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);

    static void OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailureFn(void * context, const ScopedNodeId & peerId, CHIP_ERROR error);

    static void OnSuccessReadCurrentFabricIndex(void * context, FabricIndex fabricIndex);
    static void OnReadAttributeFailure(void * context, CHIP_ERROR error);

    static void
    OnSuccessRemoveFabric(void * context,
                          const chip::app::Clusters::OperationalCredentials::Commands::NOCResponse::DecodableType & data);
    static void OnCommandFailure(void * context, CHIP_ERROR error);

    static void FinishRemoveCurrentFabric(void * context, CHIP_ERROR err);
};

/**
 * A helper class that can be used by consumers that don't care about the callback from the
 * remove fabric process and just want automatic cleanup of the CurrentFabricRemover when done
 * with it.
 */
class AutoCurrentFabricRemover : private CurrentFabricRemover
{
public:
    // Takes the same arguments as CurrentFabricRemover::RemoveCurrentFabric except without the callback.
    static CHIP_ERROR RemoveCurrentFabric(DeviceController * controller, NodeId remoteNodeId);

private:
    AutoCurrentFabricRemover(DeviceController * controller);
    static void OnRemoveCurrentFabric(void * context, NodeId remoteNodeId, CHIP_ERROR status);
    chip::Callback::Callback<OnCurrentFabricRemove> mOnRemoveCurrentFabricCallback;
};

} // namespace Controller
} // namespace chip
