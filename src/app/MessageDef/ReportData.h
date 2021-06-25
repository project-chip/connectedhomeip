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
 *      This file defines ReportData parser and builder in CHIP interaction model
 *
 */

#pragma once

#include <app/AppBuildConfig.h>
#include <app/util/basic-types.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include "AttributeDataList.h"
#include "Builder.h"
#include "EventList.h"
#include "Parser.h"

namespace chip {
namespace app {
namespace ReportData {
enum
{
    kCsTag_SuppressResponse    = 0,
    kCsTag_SubscriptionId      = 1,
    kCsTag_AttributeDataList   = 2,
    kCsTag_EventDataList       = 3,
    kCsTag_MoreChunkedMessages = 4,
};

class Parser : public chip::app::Parser
{
public:
    /**
     *  @brief Initialize the parser object with TLVReader
     *
     *  @param [in] aReader A pointer to a TLVReader, which should point to the beginning of this ReportData
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    /**
     *  @brief Roughly verify the message is correctly formed
     *   1) all mandatory tags are present
     *   2) all elements have expected data type
     *   3) any tag can only appear once
     *   4) At the top level of the structure, unknown tags are ignored for forward compatibility
     *  @note The main use of this function is to print out what we're
     *    receiving during protocol development and debugging.
     *    The encoding rule has changed in IM encoding spec so this
     *    check is only "roughly" conformant now.
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR CheckSchemaValidity() const;
#endif

    /**
     *  @brief Check whether a response (a StatusReponse specifically) is to be sent back to the request.
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
    CHIP_ERROR GetSubscriptionId(uint64_t * const apSubscriptionId) const;

    /**
     *  @brief Get a TLVReader for the AttributesDataList. Next() must be called before accessing them.
     *
     *  @param [in] apAttributeDataList    A pointer to apAttributeDataList
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a Array
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetAttributeDataList(AttributeDataList::Parser * const apAttributeDataList) const;

    /**
     *  @brief Get a TLVReader for the EventDataList. Next() must be called before accessing them.
     *
     *  @param [in] apEventDataList    A pointer to apEventDataList
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a Array
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEventDataList(EventList::Parser * const apEventDataList) const;

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

class Builder : public chip::app::Builder
{
public:
    /**
     *  @brief Initialize a ReportData::Builder for writing into a TLV stream
     *
     *  @param [in] apWriter    A pointer to TLVWriter
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    /**
     *  @brief Inject SuppressResponse into the TLV stream to indicate whether a response (a StatusResponse specifically)
     *  is to be sent back to the request.
     *
     *  @param [in] aSuppressResponse The boolean variable to indicate if request response is needed.
     *
     *  @return A reference to *this
     */
    ReportData::Builder & SuppressResponse(const bool aSuppressResponse);

    /**
     *  @brief Inject subscription id into the TLV stream, This field contains the Subscription ID
     *  to which the data is being sent against. This is not present when the ReportDataRequest is
     *  sent in response to a ReadRequest, but is present when sent in response to a SubscribeRequest.
     *  Attempts should be made to ensure the SubscriptionId does not collide with IDs from previous
     *  subscriptions to ensure disambiguation.
     *
     *  @param [in] aSubscriptionId  Subscription Id for this report data
     *
     *  @return A reference to *this
     */
    ReportData::Builder & SubscriptionId(const uint64_t aSubscriptionId);

    /**
     *  @brief Initialize a AttributeDataList::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributeDataList::Builder
     */
    AttributeDataList::Builder & CreateAttributeDataListBuilder();

    /**
     *  @brief Initialize a EventList::Builder for writing into the TLV stream
     *
     *  @return A reference to EventList::Builder
     */
    EventList::Builder & CreateEventDataListBuilder();

    /**
     *  @brief This flag is set to ‘true’ when there are more chunked messages in a transaction.
     *  @param [in] aMoreChunkedMessages The boolean variable to indicate if there are more chunked messages in a transaction.
     *  @return A reference to *this
     */
    ReportData::Builder & MoreChunkedMessages(const bool aMoreChunkedMessages);

    /**
     *  @brief Mark the end of this ReportData
     *
     *  @return A reference to *this
     */
    ReportData::Builder & EndOfReportData();

private:
    AttributeDataList::Builder mAttributeDataListBuilder;
    EventList::Builder mEventDataListBuilder;
};
}; // namespace ReportData

}; // namespace app
}; // namespace chip
