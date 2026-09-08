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

#include <app/ConcreteCommandPath.h>
#include <app/MessageDef/StatusIB.h>
#include <app/OperationalSessionSetup.h>
#include <controller/InvokeInteraction.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CancelableOperation.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>
#include <lib/core/Optional.h>
#include <system/SystemClock.h>

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

/**
 * A ControllerOperation that invokes a single command on the node it connects to. Subclasses send
 * their command from OnConnected() via Invoke(), which targets the endpoint given to Start().
 *
 * The invocation is tied to the lifetime of the operation: it is cancelled if the operation is
 * cancelled or completed before the response arrives, so nothing is left pointing at the operation.
 */
class ControllerInvokeOperationBase : public ControllerOperationBase
{
protected:
    void Start(DeviceController & controller, NodeId nodeId, EndpointId endpoint, Callback::Cancelable::Owned onCompletion);

    /**
     * Sends the given request to the endpoint passed to Start(), and hands the outcome to one of
     * the two handlers, which are free to complete (and thereby reuse or destroy) the operation.
     * An error return means neither handler will be called.
     *
     * Note the handlers run with the invocation already released: cancelling one deletes the
     * CommandSender, which must not happen from within its own callback. The CommandSender tears
     * itself down as the callback returns, so there is nothing left to cancel at that point anyway.
     */
    template <typename RequestType, typename OnSuccess, typename OnFailure>
    CHIP_ERROR Invoke(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & session, const RequestType & request,
                      OnSuccess onSuccess, OnFailure onFailure, const Optional<uint16_t> & timedInvokeTimeoutMs = NullOptional,
                      const Optional<System::Clock::Timeout> & responseTimeout = NullOptional)
    {
        using ResponseType = typename RequestType::ResponseType;
        return InvokeCommandRequest(
            &exchangeMgr, session, mEndpoint, request,
            [this, onSuccess](const app::ConcreteCommandPath & path, const app::StatusIB & status, const ResponseType & response) {
                mCancelInvoke = nullptr;
                onSuccess(path, status, response);
            },
            [this, onFailure](CHIP_ERROR error) {
                mCancelInvoke = nullptr;
                onFailure(error);
            },
            timedInvokeTimeoutMs, responseTimeout, &mCancelInvoke);
    }

    void OnFinished(bool cancelled) override;

private:
    Internal::InvokeCancelFn mCancelInvoke;
    EndpointId mEndpoint = kInvalidEndpointId;
};

template <typename... Args>
using ControllerInvokeOperation = Callback::TypedOperation<ControllerInvokeOperationBase, Args...>;

} // namespace Controller
} // namespace chip
