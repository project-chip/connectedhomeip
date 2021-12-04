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
#include <app/InteractionModelDelegate.h>
#include <app/data-model/Decode.h>
#include <functional>

namespace chip {
namespace Controller {

/*
 * This provides an adapter class that implements InteractionModelEngine::InteractionModelDelegate and provides two additional
 * features:
 *  1. The ability to pass in std::function closures to permit more flexible programming scenarios than are provided by the strict
 *     delegate interface stipulated by by InteractionModelDelegate.
 *
 *  2. Automatic decoding of attribute data provided in the TLVReader by InteractionModelDelegate::OnReportData into a decoded
 *     cluster object.
 */
template <typename DecodableAttributeType>
class TypedReadAttributeCallback final : public app::ReadClient::Callback
{
public:
    using OnSuccessCallbackType =
        std::function<void(const app::ConcreteAttributePath & aPath, const DecodableAttributeType & aData)>;
    using OnErrorCallbackType = std::function<void(const app::ConcreteAttributePath * aPath,
                                                   Protocols::InteractionModel::Status aIMStatus, CHIP_ERROR aError)>;
    using OnDoneCallbackType  = std::function<void(app::ReadClient * client, TypedReadAttributeCallback * callback)>;

    TypedReadAttributeCallback(ClusterId aClusterId, AttributeId aAttributeId, OnSuccessCallbackType aOnSuccess,
                               OnErrorCallbackType aOnError, OnDoneCallbackType aOnDone) :
        mClusterId(aClusterId),
        mAttributeId(aAttributeId), mOnSuccess(aOnSuccess), mOnError(aOnError), mOnDone(aOnDone), mBufferedReadAdapter(*this)
    {}

    app::BufferedReadCallback & GetBufferedCallback() { return mBufferedReadAdapter; }

private:
    void OnAttributeData(const app::ReadClient * apReadClient, const app::ConcreteDataAttributePath & aPath,
                         TLV::TLVReader * apData, const app::StatusIB & aStatus) override
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        DecodableAttributeType value;

        //
        // We shouldn't be getting list item operations in the provided path since that should be handled by the buffered read
        // callback. If we do, that's a bug.
        //
        VerifyOrDie(!aPath.IsListItemOperation());

        VerifyOrExit(aStatus.mStatus == Protocols::InteractionModel::Status::Success, err = CHIP_ERROR_IM_STATUS_CODE_RECEIVED);
        VerifyOrExit(aPath.mClusterId == mClusterId && aPath.mAttributeId == mAttributeId, err = CHIP_ERROR_SCHEMA_MISMATCH);
        VerifyOrExit(apData != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

        err = app::DataModel::Decode(*apData, value);
        SuccessOrExit(err);

        mOnSuccess(aPath, value);

    exit:
        if (err != CHIP_NO_ERROR)
        {
            //
            // Override aStatus to indicate an error if something bad happened above.
            //
            Protocols::InteractionModel::Status imStatus = aStatus.mStatus;
            if (aStatus.mStatus == Protocols::InteractionModel::Status::Success)
            {
                imStatus = Protocols::InteractionModel::Status::Failure;
            }

            mOnError(&aPath, imStatus, err);
        }
    }

    void OnError(const app::ReadClient * apReadClient, CHIP_ERROR aError) override
    {
        mOnError(nullptr, Protocols::InteractionModel::Status::Failure, aError);
    }

    void OnDone(app::ReadClient * apReadClient) override { mOnDone(apReadClient, this); }

    ClusterId mClusterId;
    AttributeId mAttributeId;
    OnSuccessCallbackType mOnSuccess;
    OnErrorCallbackType mOnError;
    OnDoneCallbackType mOnDone;
    app::BufferedReadCallback mBufferedReadAdapter;
};

template <typename DecodableEventType>
class TypedReadEventCallback final : public app::ReadClient::Callback
{
public:
    using OnSuccessCallbackType = std::function<void(const app::EventHeader & aEventHeader, const DecodableEventType & aData)>;
    using OnErrorCallbackType   = std::function<void(const app::EventHeader * apEventHeader,
                                                   Protocols::InteractionModel::Status aIMStatus, CHIP_ERROR aError)>;
    using OnDoneCallbackType    = std::function<void(app::ReadClient * client, TypedReadEventCallback * callback)>;

    TypedReadEventCallback(OnSuccessCallbackType aOnSuccess, OnErrorCallbackType aOnError, OnDoneCallbackType aOnDone) :
        mOnSuccess(aOnSuccess), mOnError(aOnError), mOnDone(aOnDone)
    {}

private:
    void OnEventData(const app::ReadClient * apReadClient, const app::EventHeader & aEventHeader, TLV::TLVReader * apData,
                     const app::StatusIB * apStatus) override
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
            mOnError(&aEventHeader, Protocols::InteractionModel::Status::Failure, err);
        }
    }

    void OnError(const app::ReadClient * apReadClient, CHIP_ERROR aError) override
    {
        mOnError(nullptr, Protocols::InteractionModel::Status::Failure, aError);
    }

    void OnDone(app::ReadClient * apReadClient) override { mOnDone(apReadClient, this); }

    OnSuccessCallbackType mOnSuccess;
    OnErrorCallbackType mOnError;
    OnDoneCallbackType mOnDone;
};

} // namespace Controller
} // namespace chip
