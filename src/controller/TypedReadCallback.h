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
 * cluster object.
 */
template <typename AttributeTypeInfo>
class TypedReadCallback final : public app::InteractionModelDelegate
{
public:
    using OnSuccessCallbackType =
        std::function<void(const app::ConcreteAttributePath & aPath, const typename AttributeTypeInfo::DecodableType & aData)>;
    using OnErrorCallbackType = std::function<void(const app::ConcreteAttributePath * aPath,
                                                   Protocols::InteractionModel::Status aIMStatus, CHIP_ERROR aError)>;
    using OnDoneCallbackType  = std::function<void(app::ReadClient * client, TypedReadCallback * callback)>;

    TypedReadCallback(OnSuccessCallbackType aOnSuccess, OnErrorCallbackType aOnError, OnDoneCallbackType aOnDone) :
        mOnSuccess(aOnSuccess), mOnError(aOnError), mOnDone(aOnDone)
    {}

private:
    void OnReportData(const app::ReadClient * apReadClient, const app::ClusterInfo & aPath, TLV::TLVReader * apData,
                      Protocols::InteractionModel::Status status) override
    {
        CHIP_ERROR err                           = CHIP_NO_ERROR;
        app::ConcreteAttributePath attributePath = { aPath.mEndpointId, aPath.mClusterId, aPath.mFieldId };
        typename AttributeTypeInfo::DecodableType value;

        VerifyOrExit(status == Protocols::InteractionModel::Status::Success, err = CHIP_ERROR_IM_STATUS_CODE_RECEIVED);
        VerifyOrExit(aPath.mClusterId == AttributeTypeInfo::GetClusterId() && aPath.mFieldId == AttributeTypeInfo::GetAttributeId(),
                     CHIP_ERROR_SCHEMA_MISMATCH);
        VerifyOrExit(apData != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

        err = app::DataModel::Decode(*apData, value);
        SuccessOrExit(err);

        mOnSuccess(attributePath, value);

    exit:
        if (err != CHIP_NO_ERROR)
        {
            //
            // Override status to indicate an error if something bad happened above.
            //
            if (status == Protocols::InteractionModel::Status::Success)
            {
                status = Protocols::InteractionModel::Status::Failure;
            }

            mOnError(&attributePath, status, err);
        }
    }

    CHIP_ERROR ReadError(app::ReadClient * apReadClient, CHIP_ERROR aError) override
    {
        mOnError(nullptr, Protocols::InteractionModel::Status::Failure, aError);
        mOnDone(apReadClient, this);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ReadDone(app::ReadClient * apReadClient) override
    {
        mOnDone(apReadClient, this);
        return CHIP_NO_ERROR;
    }

    OnSuccessCallbackType mOnSuccess;
    OnErrorCallbackType mOnError;
    OnDoneCallbackType mOnDone;
};

} // namespace Controller
} // namespace chip
