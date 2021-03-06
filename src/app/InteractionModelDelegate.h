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
 *      This file defines the delegate SDK consumer needs to implement, which can talk to interaction model.
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
 *   This class defines the API for a delegate that an SDK consumer can use to interface with the interaction model.
 */
class InteractionModelDelegate
{
public:
    /**
     * Notification that the interaction model has received a list of events in response to a Read request and that list
     * of events needs to be processed.
     * @param[in]  apExchangeContext   An exchange context that represents the exchange the Report Data came in on.
     *                                 It is managed externally and should not be closed by SDK consumer.
     * @param[in]  apEventListReader  TLV reader positioned at the list that contains the events.  The
     *                                implementation of EventStreamReceived is expected to call Next() on the reader to
     *                                advance it to the first element of the list, then process the elements from beginning to the end
     *                                The callee is expected to consume all events.
     *
     * @retval  # CHIP_ERROR_NOT_IMPLEMENTED if not implemented
     */
    virtual CHIP_ERROR EventStreamReceived(Messaging::ExchangeContext * apExchangeContext,
                                                         TLV::TLVReader * apEventListReader)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    /**
     * IM read client notify SDK consumer that last report in this transaction have been received and processed.
     *  and this is the last callback for this transaction
     * @retval # CHIP_ERROR_NOT_IMPLEMENTED if not implemented
     */
    virtual CHIP_ERROR ReportProcessed() { return CHIP_ERROR_NOT_IMPLEMENTED; }

    virtual ~InteractionModelDelegate() = default;
};

} // namespace app
} // namespace chip
