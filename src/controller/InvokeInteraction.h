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

#include <controller/CommandSenderAllocator.h>
#include <controller/TypedCommandCallback.h>

namespace chip {
namespace Controller {

/*
 * A typed command invocation function that takes as input a cluster-object representation of a command request and
 * callbacks for success and failure and either returns a decoded cluster-object representation of the response through
 * the provided success callback or calls the provided failure callback.
 *
 * The RequestObjectT is generally expected to be a ClusterName::Commands::CommandName::Type struct, but any object
 * that can be encoded using the DataModel::Encode machinery and exposes the GetClusterId() and GetCommandId() functions
 * is expected to work.
 *
 * The ResponseObjectT is expected to be one of two things:
 *
 *    - If a data response is expected on success, a struct type decodable via DataModel::Decode which has GetClusterId() and
 * GetCommandId() methods.  A ClusterName::Commands::ResponseCommandName::DecodableType is typically used.
 *    - If a status response is expected on success, a DataModel::NullObjectType.
 *
 */
template <typename ResponseObjectT = app::DataModel::NullObjectType, typename RequestObjectT>
CHIP_ERROR InvokeCommandRequest(Messaging::ExchangeManager * aExchangeMgr, SessionHandle sessionHandle, chip::EndpointId endpointId,
                                const RequestObjectT & requestCommandData,
                                typename TypedCommandCallback<ResponseObjectT>::OnSuccessCallbackType onSuccessCb,
                                typename TypedCommandCallback<ResponseObjectT>::OnErrorCallbackType onErrorCb)
{
    app::CommandPathParams commandPath = { endpointId, 0, RequestObjectT::GetClusterId(), RequestObjectT::GetCommandId(),
                                           (app::CommandPathFlags::kEndpointIdValid) };

    //
    // Let's create a handle version of the decoder to ensure we do correct clean-up of it if things go south at any point below
    //
    auto decoder = chip::Platform::MakeUnique<TypedCommandCallback<ResponseObjectT>>(onSuccessCb, onErrorCb);
    VerifyOrReturnError(decoder != nullptr, CHIP_ERROR_NO_MEMORY);

    //
    // Upon successful completion of SendCommandRequest below, we're expected to free up the respective allocated objects
    // in the OnDone callback.
    //
    auto onDone = [rawDecoderPtr = decoder.get()](app::CommandSender * commandSender) {
        chip::Platform::Delete(commandSender);
        chip::Platform::Delete(rawDecoderPtr);
    };

    decoder->SetOnDoneCallback(onDone);

    auto commandSender = chip::Platform::MakeUnique<app::CommandSender>(decoder.get(), aExchangeMgr);
    VerifyOrReturnError(commandSender != nullptr, CHIP_ERROR_NO_MEMORY);

    ReturnErrorOnFailure(commandSender->AddRequestData(commandPath, requestCommandData));
    ReturnErrorOnFailure(commandSender->SendCommandRequest(sessionHandle.GetPeerNodeId(), sessionHandle.GetFabricIndex(),
                                                           Optional<SessionHandle>(sessionHandle)));

    //
    // We've effectively transfered ownership of the above allocated objects to CommandSender, and we need to wait for it to call us
    // back when processing is completed (through OnDone) to eventually free up resources.
    //
    // So signal that by releasing the smart pointer.
    //
    decoder.release();
    commandSender.release();

    return CHIP_NO_ERROR;
}

} // namespace Controller
} // namespace chip
