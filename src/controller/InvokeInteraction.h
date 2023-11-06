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
#include <lib/core/Optional.h>

namespace chip {
namespace Controller {

/*
 * A typed command invocation function that takes as input a cluster-object representation of a command request and
 * callbacks for success and failure and either returns a decoded cluster-object representation of the response through
 * the provided success callback or calls the provided failure callback.
 *
 * The RequestObjectT is generally expected to be a ClusterName::Commands::CommandName::Type struct, but any object
 * that can be encoded using the DataModel::Encode machinery and exposes the
 * GetClusterId() and GetCommandId() functions and a ResponseType type
 * is expected to work.
 *
 * The ResponseType is expected to be one of two things:
 *
 *    - If a data response is expected on success, a struct type decodable via DataModel::Decode which has GetClusterId() and
 * GetCommandId() methods.  A ClusterName::Commands::ResponseCommandName::DecodableType is typically used.
 *    - If a status response is expected on success, DataModel::NullObjectType.
 *
 */
template <typename RequestObjectT>
CHIP_ERROR
InvokeCommandRequest(Messaging::ExchangeManager * aExchangeMgr, const SessionHandle & sessionHandle, chip::EndpointId endpointId,
                     const RequestObjectT & requestCommandData,
                     typename TypedCommandCallback<typename RequestObjectT::ResponseType>::OnSuccessCallbackType onSuccessCb,
                     typename TypedCommandCallback<typename RequestObjectT::ResponseType>::OnErrorCallbackType onErrorCb,
                     const Optional<uint16_t> & timedInvokeTimeoutMs,
                     const Optional<System::Clock::Timeout> & responseTimeout = NullOptional)
{
    // InvokeCommandRequest expects responses, so cannot happen over a group session.
    VerifyOrReturnError(!sessionHandle->IsGroupSession(), CHIP_ERROR_INVALID_ARGUMENT);

    app::CommandPathParams commandPath = { endpointId, 0, RequestObjectT::GetClusterId(), RequestObjectT::GetCommandId(),
                                           (app::CommandPathFlags::kEndpointIdValid) };

    //
    // Let's create a handle version of the decoder to ensure we do correct clean-up of it if things go south at any point below
    //
    auto decoder = chip::Platform::MakeUnique<TypedCommandCallback<typename RequestObjectT::ResponseType>>(onSuccessCb, onErrorCb);
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

    auto commandSender =
        chip::Platform::MakeUnique<app::CommandSender>(decoder.get(), aExchangeMgr, timedInvokeTimeoutMs.HasValue());
    VerifyOrReturnError(commandSender != nullptr, CHIP_ERROR_NO_MEMORY);

    ReturnErrorOnFailure(commandSender->AddRequestData(commandPath, requestCommandData, timedInvokeTimeoutMs));
    ReturnErrorOnFailure(commandSender->SendCommandRequest(sessionHandle, responseTimeout));

    //
    // We've effectively transferred ownership of the above allocated objects to CommandSender, and we need to wait for it to call
    // us back when processing is completed (through OnDone) to eventually free up resources.
    //
    // So signal that by releasing the smart pointer.
    //
    decoder.release();
    commandSender.release();

    return CHIP_NO_ERROR;
}

/*
 * A typed group command invocation function that takes as input a cluster-object representation of a command request.
 *
 * The RequestObjectT is generally expected to be a ClusterName::Commands::CommandName::Type struct, but any object
 * that can be encoded using the DataModel::Encode machinery and exposes the GetClusterId() and GetCommandId() functions
 * and a ResponseType type is expected to work.
 *
 * Since this sends a group command, no response will be received and all allocated resources will be cleared before exiting this
 * function
 */
template <typename RequestObjectT>
CHIP_ERROR InvokeGroupCommandRequest(Messaging::ExchangeManager * exchangeMgr, chip::FabricIndex fabric, chip::GroupId groupId,
                                     const RequestObjectT & requestCommandData)
{
    app::CommandPathParams commandPath = { groupId, RequestObjectT::GetClusterId(), RequestObjectT::GetCommandId(),
                                           app::CommandPathFlags::kGroupIdValid };
    Transport::OutgoingGroupSession session(groupId, fabric);

    auto commandSender = chip::Platform::MakeUnique<app::CommandSender>(nullptr, exchangeMgr);
    VerifyOrReturnError(commandSender != nullptr, CHIP_ERROR_NO_MEMORY);

    ReturnErrorOnFailure(commandSender->AddRequestData(commandPath, requestCommandData));
    return commandSender->SendGroupCommandRequest(SessionHandle(session));
}

template <typename RequestObjectT>
CHIP_ERROR
InvokeCommandRequest(Messaging::ExchangeManager * exchangeMgr, const SessionHandle & sessionHandle, chip::EndpointId endpointId,
                     const RequestObjectT & requestCommandData,
                     typename TypedCommandCallback<typename RequestObjectT::ResponseType>::OnSuccessCallbackType onSuccessCb,
                     typename TypedCommandCallback<typename RequestObjectT::ResponseType>::OnErrorCallbackType onErrorCb,
                     uint16_t timedInvokeTimeoutMs, const Optional<System::Clock::Timeout> & responseTimeout = NullOptional)
{
    return InvokeCommandRequest(exchangeMgr, sessionHandle, endpointId, requestCommandData, onSuccessCb, onErrorCb,
                                MakeOptional(timedInvokeTimeoutMs), responseTimeout);
}

template <typename RequestObjectT, typename std::enable_if_t<!RequestObjectT::MustUseTimedInvoke(), int> = 0>
CHIP_ERROR
InvokeCommandRequest(Messaging::ExchangeManager * exchangeMgr, const SessionHandle & sessionHandle, chip::EndpointId endpointId,
                     const RequestObjectT & requestCommandData,
                     typename TypedCommandCallback<typename RequestObjectT::ResponseType>::OnSuccessCallbackType onSuccessCb,
                     typename TypedCommandCallback<typename RequestObjectT::ResponseType>::OnErrorCallbackType onErrorCb,
                     const Optional<System::Clock::Timeout> & responseTimeout = NullOptional)
{
    return InvokeCommandRequest(exchangeMgr, sessionHandle, endpointId, requestCommandData, onSuccessCb, onErrorCb, NullOptional,
                                responseTimeout);
}

} // namespace Controller
} // namespace chip
