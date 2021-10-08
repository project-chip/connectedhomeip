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

#include "protocols/interaction_model/Constants.h"
#include <app/CommandSender.h>
#include <app/data-model/Decode.h>
#include <app/data-model/NullObject.h>
#include <controller/CommandSenderAllocator.h>
#include <functional>

namespace chip {
namespace Controller {

/*
 * This provides an adapter class that implements the CommandSender::Callback and in turn, provides two notable features:
 *  1. The ability to pass in std::function closures to permit more flexible programming scenarios than are provided by the strict
 *     delegate interface stipulated by CommandSender::Callback
 *
 *  2. Automatic decoding of command response data provided in the TLVReader by the CommandSender callback into a decoded cluster
 * object.
 */
template <typename CommandResponseObjectT>
class CommandResponseDecoder final : public app::CommandSender::Callback
{
public:
    using OnSuccessCallbackType = std::function<void(const app::ConcreteCommandPath &, const CommandResponseObjectT &)>;
    using OnErrorCallbackType   = std::function<void(Protocols::InteractionModel::Status aIMStatus, CHIP_ERROR aError)>;
    using OnDoneCallbackType    = std::function<void(app::CommandSender * commandSender)>;

    /*
     * Constructor that takes in success, failure and onDone callbacks.
     *
     * The latter can be provided later through the SetOnDoneCallback below in cases where the
     * CommandResponseDecoder object needs to be created first before it can be passed in as a closure
     * into a hypothetical OnDoneCallback function.
     */
    CommandResponseDecoder(OnSuccessCallbackType aOnSuccess, OnErrorCallbackType aOnError, OnDoneCallbackType aOnDone = {}) :
        mOnSuccess(aOnSuccess), mOnError(aOnError), mOnDone(aOnDone)
    {}

    void SetOnDoneCallback(OnDoneCallbackType callback) { mOnDone = callback; }

private:
    void OnResponse(app::CommandSender * apCommandSender, const app::ConcreteCommandPath & aCommandPath,
                    TLV::TLVReader * aReader) final;

    void OnError(const app::CommandSender * apCommandSender, Protocols::InteractionModel::Status aIMStatus, CHIP_ERROR aError)
    {
        mOnError(aIMStatus, aError);
    }

    void OnDone(app::CommandSender * apCommandSender) { mOnDone(apCommandSender); }

    OnSuccessCallbackType mOnSuccess;
    OnErrorCallbackType mOnError;
    OnDoneCallbackType mOnDone;
};

/*
 * Decodes the data provided by the TLVReader into the templated cluster object that denotes the command response.
 *
 * This function specifically decodes command responses that have actual data payloads.
 */
template <typename CommandResponseObjectT>
void CommandResponseDecoder<CommandResponseObjectT>::OnResponse(app::CommandSender * apCommandSender,
                                                                const app::ConcreteCommandPath & aCommandPath,
                                                                TLV::TLVReader * aReader)
{
    CommandResponseObjectT response;
    CHIP_ERROR err;

    if (aReader == nullptr)
    {
        mOnError(Protocols::InteractionModel::Status::Failure, CHIP_ERROR_SCHEMA_MISMATCH);
    }
    else
    {
        err = app::DataModel::Decode(*aReader, response);
        if (err != CHIP_NO_ERROR)
        {
            mOnError(Protocols::InteractionModel::Status::Failure, err);
            return;
        }
        else
        {
            mOnSuccess(aCommandPath, response);
        }
    }
}

/*
 * Decodes the data provided by the TLVReader into the templated cluster object that denotes the command response.
 *
 * This function specifically decodes command responses that do not have actual data payloads and where the passed in TLVReader
 * should be null.
 */
template <>
inline void CommandResponseDecoder<app::DataModel::NullObjectType>::OnResponse(app::CommandSender * apCommandSender,
                                                                               const app::ConcreteCommandPath & aCommandPath,
                                                                               TLV::TLVReader * aReader)
{
    if (aReader != nullptr)
    {
        mOnError(Protocols::InteractionModel::Status::Failure, CHIP_ERROR_SCHEMA_MISMATCH);
    }
    else
    {
        app::DataModel::NullObjectType nullResp;
        mOnSuccess(aCommandPath, nullResp);
    }
}

/*
 * Main function that utilizes the above constructs to invoke a command request (given a cluster-object version of it) as well as
 * callbacks for success and failure, and returns back the decoded response through the success callback.
 */
template <typename RequestType, typename ResponseType>
CHIP_ERROR InvokeCommandRequest(Messaging::ExchangeManager * aExchangeMgr, SessionHandle & sessionHandle,
                                chip::EndpointId endpointId, RequestType & requestCommandData,
                                typename CommandResponseDecoder<ResponseType>::OnSuccessCallbackType onSuccessCb,
                                typename CommandResponseDecoder<ResponseType>::OnErrorCallbackType onErrorCb)
{
    using ResponseDecoderHandle = PlatformAllocatedObjectHandle<CommandResponseDecoder<ResponseType>>;

    app::CommandPathParams commandPath = { endpointId, 0, RequestType::GetClusterId(), RequestType::GetCommandId(),
                                           (app::CommandPathFlags::kEndpointIdValid) };

    //
    // Let's create a handle version of the decoder to ensure we do correct clean-up of it if things go south at any point below
    //
    auto decoderHandle = ResponseDecoderHandle(Platform::New<CommandResponseDecoder<ResponseType>>(onSuccessCb, onErrorCb));
    auto pDecoder      = decoderHandle.get();

    VerifyOrReturnError(pDecoder != nullptr, CHIP_ERROR_NO_MEMORY);

    //
    // Upon successful completion of SendCommandRequest below, we're expected to free up the respective allocated objects
    // in the OnDone callback.
    //
    auto onDone = [pDecoder](app::CommandSender * commandSender) {
        chip::Platform::Delete(commandSender);
        chip::Platform::Delete(pDecoder);
    };

    decoderHandle->SetOnDoneCallback(onDone);

    auto commandSenderHandle = CommandSenderHandle(Platform::New<app::CommandSender>(pDecoder, aExchangeMgr));
    auto pCommandSender      = commandSenderHandle.get();
    VerifyOrReturnError(pCommandSender != nullptr, CHIP_ERROR_NO_MEMORY);

    ReturnErrorOnFailure(pCommandSender->AddRequestData(commandPath, requestCommandData));
    ReturnErrorOnFailure(pCommandSender->SendCommandRequest(sessionHandle.GetPeerNodeId(), sessionHandle.GetFabricIndex(),
                                                            Optional<SessionHandle>(sessionHandle)));

    //
    // We've effectively transfered ownership of the above allocated objects to CommandSender, and we need to wait for it to call us
    // back when processing is completed (through OnDone) to eventually free up resources.
    //
    // So signal that by releasing the smart pointer.
    //
    decoderHandle.release();
    commandSenderHandle.release();

    return CHIP_NO_ERROR;
}

} // namespace Controller
} // namespace chip
