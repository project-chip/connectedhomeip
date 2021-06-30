/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 * @file
 *    This file contains implementations for the CallbacksMgr class. The object of this
 *    class will be used by Controller applications to interact with ZCL messages.
 *    This class provide mechanism to store callbacks for global message dispatching
 *    across the ZCL stack.
 */

#include "CHIPDeviceCallbacksMgr.h"

#include <core/CHIPCore.h>
#include <inttypes.h>

namespace {

struct ReportCallbackInfo
{
    chip::NodeId nodeId;
    chip::EndpointId endpointId;
    chip::ClusterId clusterId;
    chip::AttributeId attributeId;

    bool operator==(ReportCallbackInfo const & other)
    {
        return nodeId == other.nodeId && endpointId == other.endpointId && clusterId == other.clusterId &&
            attributeId == other.attributeId;
    }
};
} // namespace

namespace chip {
namespace app {

CHIP_ERROR CHIPDeviceCallbacksMgr::AddResponseCallback(NodeId nodeId, uint8_t sequenceNumber,
                                                       Callback::Cancelable * onSuccessCallback,
                                                       Callback::Cancelable * onFailureCallback, TLVDataFilter filter)
{
    VerifyOrReturnError(onSuccessCallback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(onFailureCallback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    ResponseCallbackInfo info = { nodeId, sequenceNumber };
    static_assert(sizeof(onSuccessCallback->mInfo) >= sizeof(info), "Callback info too large");
    memcpy(&onSuccessCallback->mInfo, &info, sizeof(info));
    memcpy(&onFailureCallback->mInfo, &info, sizeof(info));

    // If some callbacks have already been registered for the same ResponseCallbackInfo, it usually means that the response
    // has not been received for a previous command with the same sequenceNumber. Cancel the previously registered callbacks.
    CancelCallback(info, mResponsesSuccess);
    CancelCallback(info, mResponsesFailure);
    PopResponseFilter(info, nullptr);

    if (filter != nullptr)
    {
        ReturnErrorOnFailure(AddResponseFilter(info, filter));
    }
    mResponsesSuccess.Enqueue(onSuccessCallback);
    mResponsesFailure.Enqueue(onFailureCallback);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CHIPDeviceCallbacksMgr::CancelResponseCallback(NodeId nodeId, uint8_t sequenceNumber)
{
    ResponseCallbackInfo info = { nodeId, sequenceNumber };
    CancelCallback(info, mResponsesSuccess);
    CancelCallback(info, mResponsesFailure);
    PopResponseFilter(info, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CHIPDeviceCallbacksMgr::AddResponseFilter(const ResponseCallbackInfo & info, TLVDataFilter filter)
{
    constexpr ResponseCallbackInfo kEmptyInfo{ kAnyNodeId, 0 };

    for (size_t i = 0; i < kTLVFilterPoolSize; i++)
    {
        if (mTLVFilterPool[i].info == kEmptyInfo)
        {
            mTLVFilterPool[i].info   = info;
            mTLVFilterPool[i].filter = filter;
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_NO_MEMORY;
}

CHIP_ERROR CHIPDeviceCallbacksMgr::PopResponseFilter(const ResponseCallbackInfo & info, TLVDataFilter * outFilter)
{
    for (size_t i = 0; i < kTLVFilterPoolSize; i++)
    {
        if (mTLVFilterPool[i].info == info)
        {
            if (outFilter != nullptr)
            {
                *outFilter = mTLVFilterPool[i].filter;
            }
            mTLVFilterPool[i].info   = ResponseCallbackInfo{ kAnyNodeId, 0 };
            mTLVFilterPool[i].filter = nullptr;
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_KEY_NOT_FOUND;
}

CHIP_ERROR CHIPDeviceCallbacksMgr::GetResponseCallback(NodeId nodeId, uint8_t sequenceNumber,
                                                       Callback::Cancelable ** onSuccessCallback,
                                                       Callback::Cancelable ** onFailureCallback, TLVDataFilter * outFilter)
{
    ResponseCallbackInfo info = { nodeId, sequenceNumber };

    ReturnErrorOnFailure(GetCallback(info, mResponsesSuccess, onSuccessCallback));
    (*onSuccessCallback)->Cancel();

    ReturnErrorOnFailure(GetCallback(info, mResponsesFailure, onFailureCallback));
    (*onFailureCallback)->Cancel();

    if (outFilter == nullptr)
    {
        PopResponseFilter(info, nullptr);
    }
    else
    {
        ReturnErrorOnFailure(PopResponseFilter(info, outFilter));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CHIPDeviceCallbacksMgr::AddReportCallback(NodeId nodeId, EndpointId endpointId, ClusterId clusterId,
                                                     AttributeId attributeId, Callback::Cancelable * onReportCallback)
{
    VerifyOrReturnError(onReportCallback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    ReportCallbackInfo info = { nodeId, endpointId, clusterId, attributeId };
    static_assert(sizeof(onReportCallback->mInfo) >= sizeof(info), "Callback info too large");
    memcpy(&onReportCallback->mInfo, &info, sizeof(info));

    // If a callback has already been registered for the same ReportCallbackInfo, let's cancel it.
    CancelCallback(info, mReports);

    mReports.Enqueue(onReportCallback);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CHIPDeviceCallbacksMgr::GetReportCallback(NodeId nodeId, EndpointId endpointId, ClusterId clusterId,
                                                     AttributeId attributeId, Callback::Cancelable ** onReportCallback)
{
    ReportCallbackInfo info = { nodeId, endpointId, clusterId, attributeId };

    ReturnErrorOnFailure(GetCallback(info, mReports, onReportCallback));

    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
