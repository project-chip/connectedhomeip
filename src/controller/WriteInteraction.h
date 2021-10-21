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
 * An adapter callback that permits applications to provide std::function callbacks for success, error and on done.
 * This permits a slightly more flexible programming model that allows applications to pass in lambdas and bound member functions
 * as they see fit instead.
 *
 */

class WriteCallback final : public app::WriteClient::Callback
{
public:
    using OnSuccessCallbackType = std::function<void(const app::ConcreteAttributePath &)>;

    //
    // Callback to deliver any error that occurs during the write. This includes
    // errors global to the write as a whole (e.g timeout) as well as per-attribute
    // errors.
    //
    // In the latter case, path will be non-null. Otherwise, it shall be null.
    //
    using OnErrorCallbackType =
        std::function<void(const app::ConcreteAttributePath *path, app::StatusIB status, CHIP_ERROR aError)>;
    using OnDoneCallbackType = std::function<void(app::WriteClient *, WriteCallback *)>;

    WriteCallback(OnSuccessCallbackType aOnSuccess, OnErrorCallbackType aOnError, OnDoneCallbackType aOnDone) :
        mOnSuccess(aOnSuccess), mOnError(aOnError), mOnDone(aOnDone)
    {}

    void OnResponse(const app::WriteClient * apWriteClient, const app::ConcreteAttributePath & aPath,
                    app::StatusIB status) override
    {
        if (status.mStatus == Protocols::InteractionModel::Status::Success)
        {
            mOnSuccess(aPath);
        }
        else
        {
            mOnError(&aPath, status, CHIP_ERROR_IM_STATUS_CODE_RECEIVED);
        }
    }

    void OnError(const app::WriteClient * apWriteClient, CHIP_ERROR aError) override
    {
        mOnError(nullptr, app::StatusIB(Protocols::InteractionModel::Status::Failure), CHIP_ERROR_IM_STATUS_CODE_RECEIVED);
    }

    void OnDone(app::WriteClient * apWriteClient) override { mOnDone(apWriteClient, this); }

private:
    OnSuccessCallbackType mOnSuccess;
    OnErrorCallbackType mOnError;
    OnDoneCallbackType mOnDone;
};

template <typename AttributeInfo>
CHIP_ERROR WriteAttribute(Messaging::ExchangeManager * aExchangeMgr, SessionHandle sessionHandle,
                                 chip::EndpointId endpointId, const typename AttributeInfo::Type & requestCommandData,
                                 WriteCallback::OnSuccessCallbackType onSuccessCb, WriteCallback::OnErrorCallbackType onErrorCb)
{
    app::WriteClientHandle handle;

    auto onDone = [](app::WriteClient * apWriteClient, WriteCallback * callback) {
        chip::Platform::Delete(callback);
    };
    
    auto callback = Platform::MakeUnique<WriteCallback>(onSuccessCb, onErrorCb, onDone);
    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_NO_MEMORY);

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
