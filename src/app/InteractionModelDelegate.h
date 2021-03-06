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
 *      This file defines the delegate zcl/app needs to implement, which can talk to interaction model.
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
 *   This class provides a delegate implemented by zcl/app-level code
 */
class InteractionModelDelegate
{
public:
    /**
     * The zcl/app-level is expected to fill in the event paths that it wants to
     * read to.
     *
     * @param[out]  aEventPathParamsList       a list of event paths the read client is interested in, zcl/app can
     *                                         allocate path list in heap, and zcp/app would free the allocated path after read
     *                                         interaction complete
     * @param[out]  aEventPathParamsListSize   Number of event paths in mpEventPathList
     * @retval  #CHIP_NO_ERROR on success
     */
    virtual CHIP_ERROR HandlePathPrepareNeededCallBack(EventPathParams *& aEventPathParamsList, size_t & aEventPathParamsListSize)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    };
    /**
     * IM read client notify zcl/app-layer code that events can be further processed.
     * @param[in]  apExchangeContext  ExchangeContext managed externally and should not be closed by zcl/app processing.
     * @param[in]  apEventListReader  EventList tlv reader, this event list tlv reader is positioned at the outer most
     *                                level at event list container and the application is expected to call Next() on
     *                                the EventListReader to advance it to the next top most element, then process event list.
     * @retval  # CHIP_NO_ERROR on success
     *          # CHIP_ERROR_NOT_IMPLEMENTED if not implemented
     */
    virtual CHIP_ERROR HandleEventStreamReceivedCallBack(Messaging::ExchangeContext * apExchangeContext,
                                                         TLV::TLVReader * apEventListReader)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    /**
     * IM read client notify zcl/app-layer code that zcl all reports have been processed.
     * @retval  # CHIP_NO_ERROR on success
     *          # CHIP_ERROR_NOT_IMPLEMENTED if not implemented
     */
    virtual CHIP_ERROR HandleReportProcessedCallBack() { return CHIP_ERROR_NOT_IMPLEMENTED; }

    virtual ~InteractionModelDelegate() = default;
};

} // namespace app
} // namespace chip
