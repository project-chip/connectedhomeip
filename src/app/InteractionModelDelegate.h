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

#include <app/AttributePathParams.h>
#include <app/ClusterInfo.h>
#include <app/MessageDef/StatusIB.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <messaging/ExchangeContext.h>
#include <protocols/interaction_model/Constants.h>
#include <protocols/secure_channel/Constants.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace app {

static constexpr uint32_t kImMessageTimeoutMsec = 12000;

class ReadClient;
class WriteClient;
class CommandSender;
class ReadHandler;

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
     * Notification that the interaction model has received a list of attribute data in response to a Read request. apData might be
     * nullptr if status is not Status::Success.
     *
     * @param[in]  apReadClient   The read client object, the application can use GetAppIdentifier() for the read client to
     *                            distinguish different read requests.
     * @param[in]  aPath          The path of the attribute, contains node id, endpoint id, cluster id, field id etc.
     * @param[in]  apData         The attribute data TLV
     * @param[in]  status         Interaction model status code
     *
     */
    virtual void OnReportData(const ReadClient * apReadClient, const ClusterInfo & aPath, TLV::TLVReader * apData,
                              Protocols::InteractionModel::Status status)
    {}

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
    virtual CHIP_ERROR ReadError(const ReadClient * apReadClient, CHIP_ERROR aError) { return CHIP_ERROR_NOT_IMPLEMENTED; }

    /**
     * Notification that a WriteClient has received a Write Response containing a status code.
     * aAttributeIndex is processing attribute index which can identify attribute if there exists multiple attribute changes with
     * same attribute path
     */
    virtual CHIP_ERROR WriteResponseStatus(const WriteClient * apWriteClient, const StatusIB & aStatusIB,
                                           AttributePathParams & aAttributePathParams, uint8_t aAttributeIndex)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    /**
     * Notification that a Write Response has been processed and application can do further work .
     */
    virtual CHIP_ERROR WriteResponseProcessed(const WriteClient * apWriteClient) { return CHIP_ERROR_NOT_IMPLEMENTED; }

    /**
     * Notification that a Write Client has received a Write Response and fails to process a attribute data element in that
     * write response
     */
    virtual CHIP_ERROR WriteResponseProtocolError(const WriteClient * apWriteClient, uint8_t aAttributeIndex)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    /**
     * Notification that a write client encountered an asynchronous failure.
     * @param[in]  apWriteClient  A current write client which can identify the write client to the consumer, particularly
     * during multiple write interactions
     * @param[in]  aError         A error that could be CHIP_ERROR_TIMEOUT when write client fails to receive, or other error when
     *                            fail to process write response.
     * @retval # CHIP_ERROR_NOT_IMPLEMENTED if not implemented
     */
    virtual CHIP_ERROR WriteResponseError(const WriteClient * apWriteClient, CHIP_ERROR aError)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    /**
     * Notification that a Subscribe Response has been processed and application can do further work .
     */
    virtual CHIP_ERROR SubscribeResponseProcessed(const ReadClient * apReadClient) { return CHIP_ERROR_NOT_IMPLEMENTED; }

    /**
     * Notification that Subscription has been established successfully and application can do further work in handler.
     */
    virtual CHIP_ERROR SubscriptionEstablished(const ReadHandler * apReadHandler) { return CHIP_ERROR_NOT_IMPLEMENTED; }

    /**
     * Notification that Subscription has been terminated in handler side.
     */
    virtual CHIP_ERROR SubscriptionTerminated(const ReadHandler * apReadHandler) { return CHIP_ERROR_NOT_IMPLEMENTED; }

    /**
     * Notification that a read interaction was completed on the client successfully.
     * @param[in]  apReadClient  A current read client which can identify the read client to the consumer, particularly
     * during multiple read interactions
     * @param[in]  aError  notify final error regarding the current read interaction
     * @retval # CHIP_ERROR_NOT_IMPLEMENTED if not implemented
     */
    virtual CHIP_ERROR ReadDone(const ReadClient * apReadClient) { return CHIP_ERROR_NOT_IMPLEMENTED; }

    virtual ~InteractionModelDelegate() = default;
};

} // namespace app
} // namespace chip
