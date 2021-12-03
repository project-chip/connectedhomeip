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
        std::function<void(const app::ConcreteAttributePath * path, app::StatusIB status, CHIP_ERROR aError)>;
    using OnDoneCallbackType = std::function<void(app::WriteClient *)>;

    WriteCallback(OnSuccessCallbackType aOnSuccess, OnErrorCallbackType aOnError, OnDoneCallbackType aOnDone) :
        mOnSuccess(aOnSuccess), mOnError(aOnError), mOnDone(aOnDone)
    {}

    void OnResponse(const app::WriteClient * apWriteClient, const app::ConcreteAttributePath & aPath, app::StatusIB status) override
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
        mOnError(nullptr, app::StatusIB(Protocols::InteractionModel::Status::Failure), aError);
    }

    void OnDone(app::WriteClient * apWriteClient) override
    {
        if (mOnDone != nullptr)
        {
            mOnDone(apWriteClient);
        }

        // Always needs to be the last call
        chip::Platform::Delete(this);
    }

private:
    OnSuccessCallbackType mOnSuccess = nullptr;
    OnErrorCallbackType mOnError     = nullptr;
    OnDoneCallbackType mOnDone       = nullptr;
};

/**
 * Functions for writing attributes.  We have lots of different AttributeInfo
 * but a fairly small set of types that get written.  So we want to keep the
 * template on AttributeInfo very small, and put all the work in the template
 * with a small number of instantiations (one per type).
 */
template <typename AttrType>
CHIP_ERROR WriteAttribute(SessionHandle sessionHandle, chip::EndpointId endpointId, ClusterId clusterId, AttributeId attributeId,
                          const AttrType & requestData, WriteCallback::OnSuccessCallbackType onSuccessCb,
                          WriteCallback::OnErrorCallbackType onErrorCb, WriteCallback::OnDoneCallbackType onDoneCb)
{
    app::WriteClientHandle handle;

    auto callback = Platform::MakeUnique<WriteCallback>(onSuccessCb, onErrorCb, onDoneCb);
    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_NO_MEMORY);

    ReturnErrorOnFailure(app::InteractionModelEngine::GetInstance()->NewWriteClient(handle, callback.get()));
    if (sessionHandle.IsGroupSession())
    {
        ReturnErrorOnFailure(
            handle.EncodeAttributeWritePayload(chip::app::AttributePathParams(clusterId, attributeId), requestData));
    }
    else
    {
        ReturnErrorOnFailure(
            handle.EncodeAttributeWritePayload(chip::app::AttributePathParams(endpointId, clusterId, attributeId), requestData));
    }

    ReturnErrorOnFailure(handle.SendWriteRequest(sessionHandle));

    callback.release();
    return CHIP_NO_ERROR;
}

template <typename AttributeInfo>
CHIP_ERROR WriteAttribute(SessionHandle sessionHandle, chip::EndpointId endpointId,
                          const typename AttributeInfo::Type & requestData, WriteCallback::OnSuccessCallbackType onSuccessCb,
                          WriteCallback::OnErrorCallbackType onErrorCb)
{
    return WriteAttribute(sessionHandle, endpointId, AttributeInfo::GetClusterId(), AttributeInfo::GetAttributeId(), requestData,
                          onSuccessCb, onErrorCb, nullptr);
}

template <typename AttributeInfo>
CHIP_ERROR WriteAttribute(SessionHandle sessionHandle, chip::EndpointId endpointId,
                          const typename AttributeInfo::Type & requestData, WriteCallback::OnSuccessCallbackType onSuccessCb,
                          WriteCallback::OnErrorCallbackType onErrorCb, WriteCallback::OnDoneCallbackType onDoneCb)
{
    return WriteAttribute(sessionHandle, endpointId, AttributeInfo::GetClusterId(), AttributeInfo::GetAttributeId(), requestData,
                          onSuccessCb, onErrorCb, onDoneCb);
}

} // namespace Controller
} // namespace chip
