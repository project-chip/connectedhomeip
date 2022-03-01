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

#include <app/BufferedReadCallback.h>
#include <app/ConcreteAttributePath.h>
#include <app/data-model/Decode.h>
#include <functional>
#include <lib/support/CHIPMem.h>

namespace chip {
namespace Controller {

/*
 * This provides an adapter class that implements ReadClient::Callback and provides three additional
 * features:
 *  1. The ability to pass in std::function closures to permit more flexible
 *     programming scenarios than are provided by the strict delegate interface
 *     stipulated by ReadClient::Callback.
 *
 *  2. Automatic decoding of attribute data provided in the TLVReader by
 *     ReadClient::Callback::OnAttributeData into a decoded cluster object.
 *
 *  3. Automatically representing all errors as a CHIP_ERROR (which might
 *     encapsulate a StatusIB).  This could be a path-specific error or it
 *     could be a general error for the entire request; the distinction is not
 *     that important, because we only have one path involved.  If the
 *     CHIP_ERROR encapsulates a StatusIB, StatusIB::InitFromChipError can be
 *     used to extract the status.
 */
template <typename DecodableAttributeType>
class TypedReadAttributeCallback final : public app::ReadClient::Callback
{
public:
    using OnSuccessCallbackType =
        std::function<void(const app::ConcreteDataAttributePath & aPath, const DecodableAttributeType & aData)>;
    using OnErrorCallbackType = std::function<void(const app::ConcreteDataAttributePath * aPath, CHIP_ERROR aError)>;
    using OnDoneCallbackType  = std::function<void(TypedReadAttributeCallback * callback)>;
    using OnSubscriptionEstablishedCallbackType = std::function<void()>;

    TypedReadAttributeCallback(ClusterId aClusterId, AttributeId aAttributeId, OnSuccessCallbackType aOnSuccess,
                               OnErrorCallbackType aOnError, OnDoneCallbackType aOnDone,
                               OnSubscriptionEstablishedCallbackType aOnSubscriptionEstablished = nullptr) :
        mClusterId(aClusterId),
        mAttributeId(aAttributeId), mOnSuccess(aOnSuccess), mOnError(aOnError), mOnDone(aOnDone),
        mOnSubscriptionEstablished(aOnSubscriptionEstablished), mBufferedReadAdapter(*this)
    {}

    app::BufferedReadCallback & GetBufferedCallback() { return mBufferedReadAdapter; }

    void AdoptReadClient(Platform::UniquePtr<app::ReadClient> aReadClient) { mReadClient = std::move(aReadClient); }

private:
    void OnAttributeData(const app::ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
                         const app::StatusIB & aStatus) override
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        DecodableAttributeType value;

        //
        // We shouldn't be getting list item operations in the provided path since that should be handled by the buffered read
        // callback. If we do, that's a bug.
        //
        VerifyOrDie(!aPath.IsListItemOperation());

        VerifyOrExit(aStatus.IsSuccess(), err = aStatus.ToChipError());
        VerifyOrExit(aPath.mClusterId == mClusterId && aPath.mAttributeId == mAttributeId, err = CHIP_ERROR_SCHEMA_MISMATCH);
        VerifyOrExit(apData != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

        SuccessOrExit(err = app::DataModel::Decode(*apData, value));

        mOnSuccess(aPath, value);

    exit:
        if (err != CHIP_NO_ERROR)
        {
            mOnError(&aPath, err);
        }
    }

    void OnError(CHIP_ERROR aError) override { mOnError(nullptr, aError); }

    void OnDone() override { mOnDone(this); }

    void OnSubscriptionEstablished(uint64_t aSubscriptionId) override
    {
        if (mOnSubscriptionEstablished)
        {
            mOnSubscriptionEstablished();
        }
    }

    void OnDeallocatePaths(chip::app::ReadPrepareParams && aReadPrepareParams) override
    {
        VerifyOrDie(aReadPrepareParams.mAttributePathParamsListSize == 1 &&
                    aReadPrepareParams.mpAttributePathParamsList != nullptr);
        chip::Platform::Delete<app::AttributePathParams>(aReadPrepareParams.mpAttributePathParamsList);

        VerifyOrDie(aReadPrepareParams.mDataVersionFilterListSize == 1 && aReadPrepareParams.mpDataVersionFilterList != nullptr);
        chip::Platform::Delete<app::DataVersionFilter>(aReadPrepareParams.mpDataVersionFilterList);
    }

    ClusterId mClusterId;
    AttributeId mAttributeId;
    OnSuccessCallbackType mOnSuccess;
    OnErrorCallbackType mOnError;
    OnDoneCallbackType mOnDone;
    OnSubscriptionEstablishedCallbackType mOnSubscriptionEstablished;
    app::BufferedReadCallback mBufferedReadAdapter;
    Platform::UniquePtr<app::ReadClient> mReadClient;
};

template <typename DecodableEventType>
class TypedReadEventCallback final : public app::ReadClient::Callback
{
public:
    using OnSuccessCallbackType = std::function<void(const app::EventHeader & aEventHeader, const DecodableEventType & aData)>;
    using OnErrorCallbackType   = std::function<void(const app::EventHeader * apEventHeader, CHIP_ERROR aError)>;
    using OnDoneCallbackType    = std::function<void(TypedReadEventCallback * callback)>;
    using OnSubscriptionEstablishedCallbackType = std::function<void()>;

    TypedReadEventCallback(OnSuccessCallbackType aOnSuccess, OnErrorCallbackType aOnError, OnDoneCallbackType aOnDone,
                           OnSubscriptionEstablishedCallbackType aOnSubscriptionEstablished = nullptr) :
        mOnSuccess(aOnSuccess),
        mOnError(aOnError), mOnDone(aOnDone), mOnSubscriptionEstablished(aOnSubscriptionEstablished)
    {}

    void AdoptReadClient(Platform::UniquePtr<app::ReadClient> aReadClient) { mReadClient = std::move(aReadClient); }

private:
    void OnEventData(const app::EventHeader & aEventHeader, TLV::TLVReader * apData, const app::StatusIB * apStatus) override
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        DecodableEventType value;

        VerifyOrExit(apData != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

        VerifyOrExit((aEventHeader.mPath.mEventId == value.GetEventId()) && (aEventHeader.mPath.mClusterId == value.GetClusterId()),
                     CHIP_ERROR_SCHEMA_MISMATCH);
        err = app::DataModel::Decode(*apData, value);
        SuccessOrExit(err);

        mOnSuccess(aEventHeader, value);

    exit:
        if (err != CHIP_NO_ERROR)
        {
            mOnError(&aEventHeader, err);
        }
    }

    void OnError(CHIP_ERROR aError) override { mOnError(nullptr, aError); }

    void OnDone() override { mOnDone(this); }

    void OnDeallocatePaths(chip::app::ReadPrepareParams && aReadPrepareParams) override
    {
        VerifyOrDie(aReadPrepareParams.mEventPathParamsListSize == 1 && aReadPrepareParams.mpEventPathParamsList != nullptr);
        chip::Platform::Delete<app::EventPathParams>(aReadPrepareParams.mpEventPathParamsList);
    }

    void OnSubscriptionEstablished(uint64_t aSubscriptionId) override
    {
        if (mOnSubscriptionEstablished)
        {
            mOnSubscriptionEstablished();
        }
    }

    OnSuccessCallbackType mOnSuccess;
    OnErrorCallbackType mOnError;
    OnDoneCallbackType mOnDone;
    OnSubscriptionEstablishedCallbackType mOnSubscriptionEstablished;
    Platform::UniquePtr<app::ReadClient> mReadClient;
};

} // namespace Controller
} // namespace chip
