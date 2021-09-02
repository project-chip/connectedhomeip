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
 *    This file contains definitions for the CallbacksMgr class. The object of this
 *    class will be used by Controller applications to interact with ZCL messages.
 *    This class provide mechanism to store callbacks for global message dispatching
 *    across the ZCL stack.
 */

#pragma once

#include <type_traits>

#include <app/util/basic-types.h>
#include <lib/core/CHIPCallback.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/DLLUtil.h>

namespace chip {
namespace app {

#ifndef CHIP_DEVICE_CALLBACK_MANAGER_TLV_FILTER_POOL_SIZE
constexpr size_t kTLVFilterPoolSize = 32;
#else
constexpr size_t kTLVFilterPoolSize = CHIP_DEVICE_CALLBACK_MANAGER_TLV_FILTER_POOL_SIZE;
#endif

/**
 * The filter interface for processing data from TLV.
 * The caller of the function will pass the original TLV data, the original success and failure callback to this function.
 * Since success callback and failure callback contains necessary context, this function itself is stateless.
 * The possible implementation of this function might be:
 *  - Unpack the data with some schema from TLV
 *  - Call success callback and failure callback according to the result of unpack routine.
 */
using TLVDataFilter = void (*)(chip::TLV::TLVReader * data, chip::Callback::Cancelable * onSuccess,
                               chip::Callback::Cancelable * onFailure);

class DLL_EXPORT CHIPDeviceCallbacksMgr
{
public:
    CHIPDeviceCallbacksMgr(const CHIPDeviceCallbacksMgr &)  = delete;
    CHIPDeviceCallbacksMgr(const CHIPDeviceCallbacksMgr &&) = delete;
    CHIPDeviceCallbacksMgr & operator=(const CHIPDeviceCallbacksMgr &) = delete;

    static CHIPDeviceCallbacksMgr & GetInstance()
    {
        static CHIPDeviceCallbacksMgr instance;
        return instance;
    }

    CHIP_ERROR AddResponseCallback(NodeId nodeId, uint8_t sequenceNumber, Callback::Cancelable * onSuccessCallback,
                                   Callback::Cancelable * onFailureCallback, TLVDataFilter callbackFilter = nullptr);
    CHIP_ERROR CancelResponseCallback(NodeId nodeId, uint8_t sequenceNumber);
    CHIP_ERROR GetResponseCallback(NodeId nodeId, uint8_t sequenceNumber, Callback::Cancelable ** onSuccessCallback,
                                   Callback::Cancelable ** onFailureCallback, TLVDataFilter * callbackFilter = nullptr);

    CHIP_ERROR AddReportCallback(NodeId nodeId, EndpointId endpointId, ClusterId clusterId, AttributeId attributeId,
                                 Callback::Cancelable * onReportCallback);
    CHIP_ERROR GetReportCallback(NodeId nodeId, EndpointId endpointId, ClusterId clusterId, AttributeId attributeId,
                                 Callback::Cancelable ** onReportCallback);

private:
    CHIPDeviceCallbacksMgr() {}

    struct ResponseCallbackInfo
    {
        chip::NodeId nodeId;
        uint8_t sequenceNumber;

        bool operator==(ResponseCallbackInfo const & other)
        {
            return nodeId == other.nodeId && sequenceNumber == other.sequenceNumber;
        }
    };

    struct TLVFilterItem
    {
        ResponseCallbackInfo info = { kPlaceholderNodeId, 0 };
        TLVDataFilter filter      = nullptr;
    };

    template <typename T>
    CHIP_ERROR CancelCallback(T & info, Callback::CallbackDeque & queue)
    {
        Callback::Cancelable * ca = nullptr;
        CHIP_ERROR err            = GetCallback(info, queue, &ca);
        if (CHIP_NO_ERROR == err)
        {
            ca->Cancel();
            queue.Dequeue(ca);
        }

        return err;
    }

    template <typename T>
    CHIP_ERROR GetCallback(T & info, Callback::CallbackDeque & queue, Callback::Cancelable ** callback)
    {
        Callback::Cancelable * ca = &queue;
        while (ca != nullptr && ca->mNext != &queue)
        {
            T callbackInfo;
            static_assert(std::is_pod<T>::value, "Callback info must be POD");
            static_assert(sizeof(ca->mNext->mInfo) >= sizeof(callbackInfo), "Callback info too large");
            memcpy(&callbackInfo, ca->mNext->mInfo, sizeof(callbackInfo));
            if (callbackInfo == info)
            {
                *callback = ca->mNext;
                return CHIP_NO_ERROR;
            }

            ca = ca->mNext;
        }

        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    CHIP_ERROR AddResponseFilter(const ResponseCallbackInfo & info, TLVDataFilter callbackFilter);
    CHIP_ERROR PopResponseFilter(const ResponseCallbackInfo & info, TLVDataFilter * callbackFilter);

    Callback::CallbackDeque mResponsesSuccess;
    Callback::CallbackDeque mResponsesFailure;
    TLVFilterItem mTLVFilterPool[kTLVFilterPoolSize];
    Callback::CallbackDeque mReports;
};

} // namespace app
} // namespace chip
