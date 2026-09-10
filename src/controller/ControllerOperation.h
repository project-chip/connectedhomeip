/*
 *    Copyright (c) 2026 Project CHIP Authors
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
#include <lib/core/CHIPError.h>
#include <lib/core/CancelableOperation.h>
#include <lib/core/NodeId.h>

namespace chip {
namespace Controller {

class DeviceController;

class ControllerOperationBase : public Callback::CancelableOperationBase
{
protected:
    /**
     * Obtains a CASE session to the given node, reporting the outcome via OnConnected() or
     * OnConnectionFailure().
     *
     * Note that a controller unable to even attempt the connection is reported as a connection
     * failure rather than returned, since the completion is already owned by then. A subclass that
     * completes the operation from OnConnectionFailure() therefore invokes the caller's callback
     * re-entrantly from here, which may reuse or destroy the operation: nothing may touch it after
     * this returns.
     */
    void Start(DeviceController & controller, NodeId nodeId, Callback::Cancelable::Owned onCompletion);

    void OnFinished(bool cancelled) override;

    virtual void OnConnected(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle) = 0;
    virtual void OnConnectionFailure(CHIP_ERROR error)                                                      = 0;

private:
    Callback::Callback<OnDeviceConnected> mDeviceConnected{
        [](void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle) {
            static_cast<ControllerOperationBase *>(context)->OnConnected(exchangeMgr, sessionHandle);
        },
        this
    };
    Callback::Callback<OnDeviceConnectionFailure> mDeviceConnectionFailure{
        [](void * context, const ScopedNodeId & peerId, CHIP_ERROR error) {
            static_cast<ControllerOperationBase *>(context)->OnConnectionFailure(error);
        },
        this
    };
};

template <typename... Args>
using ControllerOperation = Callback::TypedOperation<ControllerOperationBase, Args...>;

} // namespace Controller
} // namespace chip
