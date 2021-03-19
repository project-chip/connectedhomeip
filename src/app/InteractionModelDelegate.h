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
 *      This file defines the delegate the SDK consumer needs to implement to receive notifications from the interaction model.
 *
 */

#pragma once

#include <core/CHIPCore.h>
#include <messaging/ExchangeContext.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace app {
class ReadClient;
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
     *                                 This can be used to recover the NodeId of the node that sent the Report Data.
     *                                 It is managed externally and should not be closed by the SDK consumer.
     * @param[in]  apEventListReader  TLV reader positioned at the list that contains the events.  The
     *                                implementation of EventStreamReceived is expected to call Next() on the reader to
     *                                advance it to the first element of the list, then process the elements from beginning to the
     *                                end. The callee is expected to consume all events.
     *
     * @retval  # CHIP_ERROR_NOT_IMPLEMENTED if not implemented
     */
    virtual CHIP_ERROR EventStreamReceived(const Messaging::ExchangeContext * apExchangeContext, TLV::TLVReader * apEventListReader)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    /**
     * Notification that the last message for a Report Data action for the given ReadClient has been received and processed.
     * @param[in]  apReadClient   A current readClient which can identify the read to the consumer, particularly during
     *                            multiple read interactions
     * @retval # CHIP_ERROR_NOT_IMPLEMENTED if not implemented
     */
    virtual CHIP_ERROR ReportProcessed(const ReadClient * apReadClient) { return CHIP_ERROR_NOT_IMPLEMENTED; }

    /**
     * Notification that a read attempt encountered an asynchronous failure.
     * @param[in]  apReadClient   A current readClient which can identify the read to the consumer, particularly during
     *                            multiple read interactions
     * @param[in]  aError         A error that could be CHIP_ERROR_TIMEOUT when read client fails to receive, or other error when
     *                            fail to process report data.
     * @retval # CHIP_ERROR_NOT_IMPLEMENTED if not implemented
     */
    virtual CHIP_ERROR ReportError(const ReadClient * apReadClient, CHIP_ERROR aError) { return CHIP_ERROR_NOT_IMPLEMENTED; }

    /**
     * Notification that a Command Send receive Status code embeded in Invoke Command Response
     * @param[in]  aGeneralCode   Status code defined by the standard
     * @param[in]  aProtocolId    Protocol Id
     * @param[in]  aProtocolCode  Detailed error information, protocol-specific.
     * @param[in]  aClusterId     Cluster identifier
     * @retval # CHIP_ERROR_NOT_IMPLEMENTED if not implemented
     */
    virtual CHIP_ERROR CommandStatus(const uint16_t aGeneralCode, const uint32_t aProtocolId, const uint16_t aProtocolCode,
                             const chip::ClusterId aClusterId) { return CHIP_ERROR_NOT_IMPLEMENTED; }

    virtual ~InteractionModelDelegate() = default;
};

} // namespace app
} // namespace chip
