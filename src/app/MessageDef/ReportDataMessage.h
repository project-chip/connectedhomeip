/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      This file defines ReportDataMessage parser and builder in CHIP interaction model
 *
 */

#pragma once

#include <app/AppConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include "AttributeReportIBs.h"
#include "EventReportIBs.h"
#include "MessageBuilder.h"
#include "MessageParser.h"

namespace chip {
namespace app {
namespace ReportDataMessage {
enum class Tag : uint8_t
{
    kSubscriptionId      = 0,
    kAttributeReportIBs  = 1,
    kEventReports        = 2,
    kMoreChunkedMessages = 3,
    kSuppressResponse    = 4,
};

class Parser : public MessageParser
{
public:
#if CHIP_CONFIG_IM_PRETTY_PRINT
    CHIP_ERROR PrettyPrint() const;
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

    /**
     *  @brief Check whether a response (a StatusResponseMessage specifically) is to be sent back to the request.
     *  Next() must be called before accessing them.
     *
     *  @param [in] apSuppressResponse    A pointer to apSuppressResponse
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetSuppressResponse(bool * const apSuppressResponse) const;

    /**
     *  @brief Get Subscription Id. Next() must be called before accessing them.
     *
     *  @param [in] apSubscriptionId    A pointer to apSubscriptionId
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetSubscriptionId(SubscriptionId * const apSubscriptionId) const;

    /**
     *  @brief Get a TLVReader for the AttributesDataList. Next() must be called before accessing them.
     *
     *  @param [in] apAttributeReportIBs    A pointer to apAttributeReportIBs
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a Array
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetAttributeReportIBs(AttributeReportIBs::Parser * const apAttributeReportIBs) const;

    /**
     *  @brief Get a TLVReader for the EventReportIBs. Next() must be called before accessing them.
     *
     *  @param [in] apEventReports    A pointer to apEventReports
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a Array
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEventReports(EventReportIBs::Parser * const apEventReports) const;

    /**
     *  @brief Check whether there are more chunked messages in a transaction. Next() must be called before accessing them.
     *
     *  @param [in] apMoreChunkedMessages   A pointer to apMoreChunkedMessages
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetMoreChunkedMessages(bool * const apMoreChunkedMessages) const;
};

class Builder : public MessageBuilder
{
public:
    /**
     *  @brief Inject SuppressResponse into the TLV stream to indicate whether a response (a StatusResponseMessage specifically)
     *  is to be sent back to the request.
     *
     *  @param [in] aSuppressResponse The boolean variable to indicate if request response is needed.
     *
     *  @return A reference to *this
     */
    ReportDataMessage::Builder & SuppressResponse(const bool aSuppressResponse);

    /**
     *  @brief Inject subscription id into the TLV stream, This field contains the Subscription ID
     *  to which the data is being sent against. This is not present when the ReportDataMessageRequest is
     *  sent in response to a ReadRequestMessage, but is present when sent in response to a SubscribeRequestMessage.
     *  Attempts should be made to ensure the SubscriptionId does not collide with IDs from previous
     *  subscriptions to ensure disambiguation.
     *
     *  @param [in] aSubscriptionId  Subscription Id for this report data
     *
     *  @return A reference to *this
     */
    ReportDataMessage::Builder & SubscriptionId(const chip::SubscriptionId aSubscriptionId);

    /**
     *  @brief Initialize a AttributeReportIBs::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributeReportIBs::Builder
     */
    AttributeReportIBs::Builder & CreateAttributeReportIBs();

    /**
     *  @brief Initialize a EventReportIBs::Builder for writing into the TLV stream
     *
     *  @return A reference to EventReportIBs::Builder
     */
    EventReportIBs::Builder & CreateEventReports();

    /**
     *  @brief This flag is set to ‘true’ when there are more chunked messages in a transaction.
     *  @param [in] aMoreChunkedMessages The boolean variable to indicate if there are more chunked messages in a transaction.
     *  @return A reference to *this
     */
    ReportDataMessage::Builder & MoreChunkedMessages(const bool aMoreChunkedMessages);

    /**
     *  @brief Mark the end of this ReportDataMessage
     *
     *  @return The builder's final status.
     */
    CHIP_ERROR EndOfReportDataMessage();

private:
    AttributeReportIBs::Builder mAttributeReportIBsBuilder;
    EventReportIBs::Builder mEventReportsBuilder;
};
} // namespace ReportDataMessage
} // namespace app
} // namespace chip
