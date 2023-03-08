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

#include <app/ChunkedWriteCallback.h>
#include <app/InteractionModelEngine.h>
#include <app/WriteClient.h>
#include <controller/CommandSenderAllocator.h>
#include <controller/TypedCommandCallback.h>
#include <lib/core/Optional.h>

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
    using OnErrorCallbackType = std::function<void(const app::ConcreteAttributePath * path, CHIP_ERROR err)>;
    using OnDoneCallbackType  = std::function<void(app::WriteClient *)>;

    WriteCallback(OnSuccessCallbackType aOnSuccess, OnErrorCallbackType aOnError, OnDoneCallbackType aOnDone, bool aIsGroupWrite) :
        mOnSuccess(aOnSuccess), mOnError(aOnError), mOnDone(aOnDone), mIsGroupWrite(aIsGroupWrite), mCallback(this)
    {}

    app::WriteClient::Callback * GetChunkedCallback() { return &mCallback; }

    void OnResponse(const app::WriteClient * apWriteClient, const app::ConcreteDataAttributePath & aPath,
                    app::StatusIB status) override
    {
        if (mCalledCallback)
        {
            return;
        }
        mCalledCallback = true;

        if (status.IsSuccess())
        {
            mOnSuccess(aPath);
        }
        else
        {
            mOnError(&aPath, status.ToChipError());
        }
    }

    void OnError(const app::WriteClient * apWriteClient, CHIP_ERROR aError) override
    {
        if (mCalledCallback)
        {
            return;
        }
        mCalledCallback = true;

        mOnError(nullptr, aError);
    }

    void OnDone(app::WriteClient * apWriteClient) override
    {
        if (!mIsGroupWrite && !mCalledCallback)
        {
            // This can happen if the server sends a response with an empty
            // WriteResponses list.  Since we are not sending wildcard write
            // paths, that's not a valid response and we should treat it as an
            // error.  Use the error we would have gotten if we in fact expected
            // a nonempty list.
            OnError(apWriteClient, CHIP_END_OF_TLV);
        }

        if (mOnDone != nullptr)
        {
            mOnDone(apWriteClient);
        }

        chip::Platform::Delete(apWriteClient);
        // Always needs to be the last call
        chip::Platform::Delete(this);
    }

private:
    OnSuccessCallbackType mOnSuccess = nullptr;
    OnErrorCallbackType mOnError     = nullptr;
    OnDoneCallbackType mOnDone       = nullptr;

    bool mCalledCallback = false;
    bool mIsGroupWrite   = false;

    app::ChunkedWriteCallback mCallback;
};

/**
 * Functions for writing attributes.  We have lots of different AttributeInfo
 * but a fairly small set of types that get written.  So we want to keep the
 * template on AttributeInfo very small, and put all the work in the template
 * with a small number of instantiations (one per type).
 */
template <typename AttrType>
CHIP_ERROR WriteAttribute(const SessionHandle & sessionHandle, chip::EndpointId endpointId, ClusterId clusterId,
                          AttributeId attributeId, const AttrType & requestData, WriteCallback::OnSuccessCallbackType onSuccessCb,
                          WriteCallback::OnErrorCallbackType onErrorCb, const Optional<uint16_t> & aTimedWriteTimeoutMs,
                          WriteCallback::OnDoneCallbackType onDoneCb = nullptr,
                          const Optional<DataVersion> & aDataVersion = NullOptional)
{
    auto callback = Platform::MakeUnique<WriteCallback>(onSuccessCb, onErrorCb, onDoneCb, sessionHandle->IsGroupSession());
    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_NO_MEMORY);

    auto client = Platform::MakeUnique<app::WriteClient>(app::InteractionModelEngine::GetInstance()->GetExchangeManager(),
                                                         callback->GetChunkedCallback(), aTimedWriteTimeoutMs);
    VerifyOrReturnError(client != nullptr, CHIP_ERROR_NO_MEMORY);

    if (sessionHandle->IsGroupSession())
    {
        ReturnErrorOnFailure(client->EncodeAttribute(chip::app::AttributePathParams(clusterId, attributeId), requestData));
    }
    else
    {
        ReturnErrorOnFailure(
            client->EncodeAttribute(chip::app::AttributePathParams(endpointId, clusterId, attributeId), requestData, aDataVersion));
    }

    ReturnErrorOnFailure(client->SendWriteRequest(sessionHandle));

    // At this point the handle will ensure our callback's OnDone is always
    // called.
    client.release();
    callback.release();

    return CHIP_NO_ERROR;
}

template <typename AttributeInfo>
CHIP_ERROR WriteAttribute(const SessionHandle & sessionHandle, chip::EndpointId endpointId,
                          const typename AttributeInfo::Type & requestData, WriteCallback::OnSuccessCallbackType onSuccessCb,
                          WriteCallback::OnErrorCallbackType onErrorCb, const Optional<uint16_t> & aTimedWriteTimeoutMs,
                          WriteCallback::OnDoneCallbackType onDoneCb = nullptr,
                          const Optional<DataVersion> & aDataVersion = NullOptional)
{
    return WriteAttribute(sessionHandle, endpointId, AttributeInfo::GetClusterId(), AttributeInfo::GetAttributeId(), requestData,
                          onSuccessCb, onErrorCb, aTimedWriteTimeoutMs, onDoneCb, aDataVersion);
}

template <typename AttributeInfo>
CHIP_ERROR WriteAttribute(const SessionHandle & sessionHandle, chip::EndpointId endpointId,
                          const typename AttributeInfo::Type & requestData, WriteCallback::OnSuccessCallbackType onSuccessCb,
                          WriteCallback::OnErrorCallbackType onErrorCb, uint16_t aTimedWriteTimeoutMs,
                          WriteCallback::OnDoneCallbackType onDoneCb = nullptr,
                          const Optional<DataVersion> & aDataVersion = NullOptional)
{
    return WriteAttribute<AttributeInfo>(sessionHandle, endpointId, requestData, onSuccessCb, onErrorCb, onDoneCb,
                                         MakeOptional(aTimedWriteTimeoutMs), onDoneCb, aDataVersion);
}

template <typename AttributeInfo, typename std::enable_if_t<!AttributeInfo::MustUseTimedWrite(), int> = 0>
CHIP_ERROR WriteAttribute(const SessionHandle & sessionHandle, chip::EndpointId endpointId,
                          const typename AttributeInfo::Type & requestData, WriteCallback::OnSuccessCallbackType onSuccessCb,
                          WriteCallback::OnErrorCallbackType onErrorCb, WriteCallback::OnDoneCallbackType onDoneCb = nullptr,
                          const Optional<DataVersion> & aDataVersion = NullOptional)
{
    return WriteAttribute<AttributeInfo>(sessionHandle, endpointId, requestData, onSuccessCb, onErrorCb, NullOptional, onDoneCb,
                                         aDataVersion);
}

} // namespace Controller
} // namespace chip
