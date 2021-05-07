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

#include <app/util/basic-types.h>
#include <core/CHIPCallback.h>
#include <core/CHIPError.h>
#include <support/DLLUtil.h>

namespace chip {
namespace app {

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
                                   Callback::Cancelable * onFailureCallback);
    CHIP_ERROR CancelResponseCallback(NodeId nodeId, uint8_t sequenceNumber);
    CHIP_ERROR GetResponseCallback(NodeId nodeId, uint8_t sequenceNumber, Callback::Cancelable ** onSuccessCallback,
                                   Callback::Cancelable ** onFailureCallback);

    CHIP_ERROR AddReportCallback(NodeId nodeId, EndpointId endpointId, ClusterId clusterId, AttributeId attributeId,
                                 Callback::Cancelable * onReportCallback);
    CHIP_ERROR GetReportCallback(NodeId nodeId, EndpointId endpointId, ClusterId clusterId, AttributeId attributeId,
                                 Callback::Cancelable ** onReportCallback);

private:
    CHIPDeviceCallbacksMgr() {}

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
            if (*reinterpret_cast<T *>(&ca->mNext->mInfoPtr) == info)
            {
                *callback = ca->mNext;
                return CHIP_NO_ERROR;
            }

            ca = ca->mNext;
        }

        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    Callback::CallbackDeque mResponsesSuccess;
    Callback::CallbackDeque mResponsesFailure;
    Callback::CallbackDeque mReports;
};

} // namespace app
} // namespace chip
