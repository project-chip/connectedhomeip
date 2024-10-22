/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "core/BaseCluster.h"
#include "core/Endpoint.h"

#include "lib/support/logging/CHIPLogging.h"

namespace matter {
namespace casting {
namespace core {
template <typename ResponseType>
using CommandSuccessCallbackType = std::function<void(void * context, const ResponseType & responseObject)>;
using CommandFailureCallbackType = std::function<void(void * context, CHIP_ERROR err)>;

template <typename RequestType>
struct CommandContext;

template <typename RequestType>
class Command
{
public:
    Command(memory::Weak<core::Endpoint> endpoint) { this->mEndpoint = endpoint; }

    /**
     * @brief Invokes this command on the associated Endpoint and corresponding Cluster
     *
     * @param request request data corresponding to this command invocation
     * @param successCb Called on command execution success, with responseData
     * @param failureCb Called on command execution failure
     * @param timedInvokeTimeoutMs command timeout
     */
    void Invoke(RequestType request, void * context, CommandSuccessCallbackType<typename RequestType::ResponseType> successCb,
                CommandFailureCallbackType failureCb, const chip::Optional<uint16_t> & timedInvokeTimeoutMs)
    {
        memory::Strong<core::Endpoint> endpoint = this->GetEndpoint().lock();
        if (endpoint)
        {
            CommandContext<RequestType> * commandContext =
                new CommandContext<RequestType>(endpoint, request, context, successCb, failureCb, timedInvokeTimeoutMs);

            endpoint->GetCastingPlayer()->FindOrEstablishSession(
                commandContext,
                // FindOrEstablishSession success handler
                [](void * _context, chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle) {
                    CommandContext<RequestType> * _commandContext = static_cast<CommandContext<RequestType> *>(_context);
                    ChipLogProgress(AppServer, "<Command>::Invoke() Found or established session");

                    // Invoke command
                    MediaClusterBase cluster(exchangeMgr, sessionHandle, _commandContext->mEndpoint->GetId());
                    CHIP_ERROR err = cluster.template InvokeCommand<RequestType>(
                        _commandContext->mRequest, _commandContext,
                        // Command success handler
                        [](void * __context, const typename RequestType::ResponseType & response) {
                            CommandContext<RequestType> * __commandContext = static_cast<CommandContext<RequestType> *>(__context);
                            ChipLogProgress(AppServer, "<Command>::Invoke() response success");
                            __commandContext->mSuccessCb(__commandContext->mClientContext, response);
                            delete __commandContext;
                        },
                        // Command failure handler
                        [](void * __context, CHIP_ERROR error) {
                            CommandContext<RequestType> * __commandContext = static_cast<CommandContext<RequestType> *>(__context);
                            ChipLogError(AppServer,
                                         "<Command>::Invoke() failure response on EndpointId: %d with error: "
                                         "%" CHIP_ERROR_FORMAT,
                                         __commandContext->mEndpoint->GetId(), error.Format());
                            __commandContext->mFailureCb(__commandContext->mClientContext, error);
                            delete __commandContext;
                        },
                        _commandContext->mTimedInvokeTimeoutMs);

                    // error in invoking the command
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(AppServer,
                                     "<Command>::Invoke() failure in invoking command on EndpointId: %d with error: "
                                     "%" CHIP_ERROR_FORMAT,
                                     _commandContext->mEndpoint->GetId(), err.Format());
                        _commandContext->mFailureCb(_commandContext->mClientContext, err);
                        delete _commandContext;
                    }
                },
                // FindOrEstablishSession failure handler
                [](void * _context, const chip::ScopedNodeId & peerId, CHIP_ERROR error) {
                    CommandContext<RequestType> * _commandContext = static_cast<CommandContext<RequestType> *>(_context);
                    ChipLogError(AppServer,
                                 "<Command>::Invoke() failure in retrieving session info for peerId.nodeId: "
                                 "0x" ChipLogFormatX64 ", peer.fabricIndex: %d with error: %" CHIP_ERROR_FORMAT,
                                 ChipLogValueX64(peerId.GetNodeId()), peerId.GetFabricIndex(), error.Format());
                    _commandContext->mFailureCb(_commandContext->mClientContext, error);
                    delete _commandContext;
                });
        }
        else
        {
            ChipLogError(AppServer, "<Command>::Invoke() failure in retrieving Endpoint");
            failureCb(context, CHIP_ERROR_INCORRECT_STATE);
        }
    }

protected:
    memory::Weak<core::Endpoint> GetEndpoint() const { return mEndpoint.lock(); }
    memory::Weak<core::Endpoint> mEndpoint;
};

/**
 * @brief Context object used by the Command class during the Invoke API's execution
 */
template <typename RequestType>
struct CommandContext
{
    CommandContext(memory::Strong<core::Endpoint> endpoint, RequestType request, void * context,
                   CommandSuccessCallbackType<typename RequestType::ResponseType> successCb, CommandFailureCallbackType failureCb,
                   const chip::Optional<uint16_t> & timedInvokeTimeoutMs) :
        mSuccessCb(successCb),
        mFailureCb(failureCb)
    {
        mEndpoint             = endpoint;
        mRequest              = request;
        mClientContext        = context;
        mTimedInvokeTimeoutMs = timedInvokeTimeoutMs;
    }

    memory::Strong<core::Endpoint> mEndpoint;
    RequestType mRequest;
    void * mClientContext;
    CommandSuccessCallbackType<typename RequestType::ResponseType> mSuccessCb;
    CommandFailureCallbackType mFailureCb;
    chip::Optional<uint16_t> mTimedInvokeTimeoutMs;
};

} // namespace core
} // namespace casting
} // namespace matter
