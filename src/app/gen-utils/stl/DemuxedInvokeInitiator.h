/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

/**
 *    @file
 *      This file defines Base class for a CHIP IM Command
 *
 */

#pragma once

#include <core/CHIPCore.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <support/BitFlags.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <InvokeInteraction.h>
#include <app/MessageDef/CommandDataElement.h>
#include <app/MessageDef/CommandList.h>
#include <app/MessageDef/InvokeCommand.h>
#include <basic-types.h>
#include <functional>
#include <protocols/secure_channel/Constants.h>
#include <vector>

namespace chip {
namespace app {

/*
 * @brief
 *
 * This class provides syntactic glue to make it easier for command initiators to register handlers for responses and errors
 * for the commands they send by handling the demuxing of received command responses and routing it to the right registered handler
 * based on the cluster and command id. This class wraps the InvokeInitiator class and provides pass-through calls to the underlying
 * object as needed to maintain the same public API as that of the InvokeInitiator class.
 *
 * At the time a command request is added, handlers for the success and error cases are passed in as well.
 *
 * To achieve a hands-free allocation model of the ensuing response object, the method to add a request is templated against the
 * type of the response, which is then allocated on stack inside an embedded closure before being passed up to the application. This
 * avoids the application having to manage the allocation of the response object and the subsequent destruction of it.
 *
 * This class can only be used with cluster objects that support the IEncodableElement interface!
 *
 * The logic below leverages std::function to provide a flexible means to express the handler callbacks, which can be used along
 * with std::bind to achieve a flexible scheme for expressing callbacks. As such, the class below is not suitable for embedded
 * platforms.
 *
 */
class DemuxedInvokeInitiator : public InvokeInitiator::ICommandHandler
{
public:
    typedef std::function<void(DemuxedInvokeInitiator & demuxedInitiator)> onDoneFuncDef;

    /*
     * @brief
     *
     * Constructor that expects to be passed an onDoneFunc that will be called when the invoke has completed
     * and the object is safe for destruction.
     */
    DemuxedInvokeInitiator(onDoneFuncDef onDoneFunc);

    /*
     * @brief
     *
     * Initializes the object by calling down to the invokeInitiator's method and passing self as the command handler
     *
     */
    CHIP_ERROR Init(Messaging::ExchangeManager * apExchangeMgr, NodeId aNodeId, Transport::AdminId aAdminId,
                    SecureSessionHandle * secureSession)
    {
        return mInitiator.Init(apExchangeMgr, this, aNodeId, aAdminId, secureSession);
    }

    /*
     * @brief
     *
     * Add a command request along with a response and error functions. This is a templated method with the type of the response
     * object is used as the specialization. The response object is allocated on stack through a small closure that is invoked when
     * the ensuing response is parsed by the OnResponse method.
     *
     */
    template <typename T>
    CHIP_ERROR AddCommand(
        IEncodableElement * request, CommandParams params,
        std::function<void(DemuxedInvokeInitiator & invokeInitiator, CommandParams &, T * t1)> onDataFunc,
        std::function<void(DemuxedInvokeInitiator & invokeInitiator, CHIP_ERROR error, StatusResponse * response)> onErrorFunc = {})
    {
        auto onDataClosure = [onDataFunc](DemuxedInvokeInitiator & initiator, CommandParams & param, TLV::TLVReader * reader) {
            if (reader)
            {
                T t1;
                t1.Decode(*reader);
                onDataFunc(initiator, param, &t1);
            }
            else
            {
                onDataFunc(initiator, param, NULL);
            }
        };

        mHandlers.push_back({ onDataClosure, onErrorFunc, T::GetClusterId(), T::GetCommandId() });
        return mInitiator.AddRequest(params, request);
    }

    /*
     * @brief
     *
     * Add a command request along with a response and error functions. Unlike the above, this does not expect a data response and
     * as such, the signature of the onDataFunc callback does not contain the response object passed in.
     *
     */
    CHIP_ERROR AddCommand(IEncodableElement * request, CommandParams params,
                          std::function<void(DemuxedInvokeInitiator & invokeInitiator, CommandParams &)> onDataFunc = {},
                          std::function<void(DemuxedInvokeInitiator & invokeInitiator, CHIP_ERROR error, StatusResponse * response)>
                              onErrorFunc = {});

    CHIP_ERROR Send() { return mInitiator.Send(); }

    InvokeInitiator & GetInitiator() { return mInitiator; }

    ~DemuxedInvokeInitiator() {}

private:
    void OnResponse(InvokeInitiator & initiator, CommandParams & params, TLV::TLVReader * payload) final;

    void OnError(InvokeInitiator & initiator, CommandParams * params, CHIP_ERROR error, StatusResponse * statusResponse) final;

    void OnEnd(InvokeInitiator & initiator) final;

    struct Handler
    {
        std::function<void(DemuxedInvokeInitiator & invokeInitiator, CommandParams & params, TLV::TLVReader * reader)>
            onDataClosure;
        std::function<void(DemuxedInvokeInitiator & invokeInitiator, CHIP_ERROR error, StatusResponse * response)> onErrorFunc;
        chip::ClusterId clusterId;
        chip::CommandId commandId;
    };

    InvokeInitiator mInitiator;
    std::vector<Handler> mHandlers;
    onDoneFuncDef mOnDoneFunc;
};

} // namespace app
} // namespace chip
