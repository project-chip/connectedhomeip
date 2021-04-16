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
#include <core/CHIPTLV.h>
#include <messaging/ExchangeContext.h>
#include <protocols/secure_channel/Constants.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace app {
class ReadClient;
class CommandSender;
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
     * Notification that a Command Send has received an Invoke Command Response containing a status code.
     * @param[in]  apCommandSender A current command sender which can identify the command sender to the consumer, particularly
     * during multiple command interactions
     * @param[in]  aGeneralCode   Status code defined by the standard
     * @param[in]  aProtocolId    Protocol Id
     * @param[in]  aProtocolCode  Detailed error information, protocol-specific.
     * @param[in]  aEndpointId    Endpoint identifier
     * @param[in]  aClusterId     Cluster identifier
     * @param[in]  aCommandId     Command identifier
     * @param[in]  aCommandIndex  Current processing command index which can identify command if there exists multiple commands with
     * same command Id
     * @retval # CHIP_ERROR_NOT_IMPLEMENTED if not implemented
     */
    virtual CHIP_ERROR CommandResponseStatus(const CommandSender * apCommandSender,
                                             const Protocols::SecureChannel::GeneralStatusCode aGeneralCode,
                                             const uint32_t aProtocolId, const uint16_t aProtocolCode, chip::EndpointId aEndpointId,
                                             const chip::ClusterId aClusterId, chip::CommandId aCommandId, uint8_t aCommandIndex)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    /**
     * Notification that a Command Response has already been processed.
     * @param[in]  apCommandSender A current command sender which can identify the command sender to the consumer, particularly
     * during multiple command interactions
     * @retval # CHIP_ERROR_NOT_IMPLEMENTED if not implemented
     */
    virtual CHIP_ERROR CommandResponseProcessed(const CommandSender * apCommandSender) { return CHIP_ERROR_NOT_IMPLEMENTED; }

    /**
     * Notification that a Command Send has received an Invoke Command Response and fails to process a command data element in that
     * command response
     * @param[in]  apCommandSender A current command sender which can identify the command sender to the consumer, particularly
     * during multiple command interactions
     * @param[in]  aCommandIndex  Current processing command index which can identify failed command
     * @retval # CHIP_ERROR_NOT_IMPLEMENTED if not implemented
     */
    virtual CHIP_ERROR CommandResponseProtocolError(const CommandSender * apCommandSender, uint8_t aCommandIndex)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    /**
     * Notification that a command sender encountered an asynchronous failure.
     * @param[in]  apCommandSender A current command sender which can identify the command sender to the consumer, particularly
     * during multiple command interactions
     * @param[in]  aError         A error that could be CHIP_ERROR_TIMEOUT when command sender fails to receive, or other error when
     *                            fail to process command response.
     * @retval # CHIP_ERROR_NOT_IMPLEMENTED if not implemented
     */
    virtual CHIP_ERROR CommandResponseError(const CommandSender * apCommandSender, CHIP_ERROR aError)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    virtual ~InteractionModelDelegate() = default;
};

} // namespace app
} // namespace chip
