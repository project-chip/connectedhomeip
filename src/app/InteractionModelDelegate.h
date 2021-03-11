/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *    @file
 *      This file defines the classes corresponding to CHIP Interaction Model Delegate to interactive with zcl/app
 *
 */

#pragma once

#include <core/CHIPCore.h>
#include <messaging/ExchangeContext.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace app {

struct EventPathParams;

/**
 * @brief
 *   This class provides a skeleton for the callback functions. The functions will be
 *   called by InteractionModel on specific CallbackId. If the user of InteractionModel
 *   is interested in receiving these callbacks, they can specialize this class and handle
 *   each CallbackId in their implementation of this class.
 */
class InteractionModelDelegate
{
public:
    /**
     * Sent when it is ready to send the ReadRequest.  The
     * application is expected to fill in the attribute/event paths that it wants to
     * read/subscribe to, this code can be potentially be reused in subscribe request, where it
     * may be called during resubscribe.
     *
     * @param[in]  aEventPathParamsList       a list of event path parameter ZCL
     * @param[in]  aEventPathParamsListSize   Number of event paths in mpEventPathList
     * @param[in]  aEventPathParamsListSize   A event number it has already to limit the set of retrieved events
     * on the server for optimization purposes
     * @param[out] CHIP_ERROR
     */
    virtual CHIP_ERROR HandlePathPrepareNeededCallBack(EventPathParams *& aEventPathParamsList, size_t & aEventPathParamsListSize,
                                                       uint64_t & aEventNumber)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    };
    /**
     * IM notify zcl events can be further processed in app.
     * @param[in]  apExchangeContext   ExchangeContext managed externally and should not be closed by zcl processing.
     * @param[in]  aEventPathParamsListSize  EventList tlv reader, upper layer can process event list.
     * @param[out] CHIP_ERROR
     */
    virtual CHIP_ERROR HandleEventStreamReceivedCallBack(Messaging::ExchangeContext * apExchangeContext,
                                                         chip::TLV::TLVReader * apReader)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    // IM notify zcl all reports have been processed.
    virtual CHIP_ERROR HandleReportProcessedCallBack() { return CHIP_ERROR_NOT_IMPLEMENTED; }

    virtual ~InteractionModelDelegate() = default;
};

} // namespace app
} // namespace chip
