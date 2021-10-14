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

#include <app/InteractionModelEngine.h>
#include <app/WriteClient.h>
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
 */

class WriteCallback final : public app::WriteClient::Callback
{
public:
    using OnSuccessCallbackType = std::function<void(const app::ConcreteAttributePath &)>;
    using OnErrorCallbackType =
        std::function<void(const app::ConcreteAttributePath *, Protocols::InteractionModel::Status, CHIP_ERROR aError)>;
    using OnDoneCallbackType = std::function<void(app::WriteClient *, WriteCallback *)>;

    WriteCallback(OnSuccessCallbackType aOnSuccess, OnErrorCallbackType aOnError, OnDoneCallbackType aOnDone) :
        mOnSuccess(aOnSuccess), mOnError(aOnError), mOnDone(aOnDone)
    {}

    void OnResponse(const app::WriteClient * apWriteClient, const app::ConcreteAttributePath & aPath,
                    Protocols::InteractionModel::Status aIMStatus) override
    {
        if (aIMStatus == Protocols::InteractionModel::Status::Success)
        {
            mOnSuccess(aPath);
        }
        else
        {
            mOnError(&aPath, aIMStatus, CHIP_ERROR_IM_STATUS_CODE_RECEIVED);
        }
    }

    void OnError(const app::WriteClient * apWriteClient, CHIP_ERROR aError) override
    {
        mOnError(nullptr, Protocols::InteractionModel::Status::Failure, CHIP_ERROR_IM_STATUS_CODE_RECEIVED);
    }

    void OnDone(app::WriteClient * apWriteClient) override { mOnDone(apWriteClient, this); }

private:
    OnSuccessCallbackType mOnSuccess;
    OnErrorCallbackType mOnError;
    OnDoneCallbackType mOnDone;
};

template <typename AttributeInfo>
CHIP_ERROR WriteAttributeRequest(Messaging::ExchangeManager * aExchangeMgr, SessionHandle sessionHandle,
                                 chip::EndpointId endpointId, const typename AttributeInfo::Type & requestCommandData,
                                 WriteCallback::OnSuccessCallbackType onSuccessCb, WriteCallback::OnErrorCallbackType onErrorCb)
{
    app::WriteClientHandle handle;

    auto callback = Platform::MakeUnique<WriteCallback>(
        onSuccessCb, onErrorCb, [](app::WriteClient * apWriteClient, WriteCallback * this_) { Platform::Delete(this_); });

    ReturnErrorOnFailure(app::InteractionModelEngine::GetInstance()->NewWriteClient(handle, callback.get()));
    ReturnErrorOnFailure(handle.EncodeAttributeWritePayload(
        chip::app::AttributePathParams(endpointId, AttributeInfo::GetClusterId(), AttributeInfo::GetAttributeId()),
        requestCommandData));
    ReturnErrorOnFailure(handle.SendWriteRequest(sessionHandle.GetPeerNodeId(), sessionHandle.GetFabricIndex(),
                                                 chip::Optional<chip::SessionHandle>(sessionHandle)));

    callback.release();
    return CHIP_NO_ERROR;
}

} // namespace Controller
} // namespace chip
