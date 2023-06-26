/**
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
 *      This file defines WriteRequestMessage parser and builder in CHIP interaction model
 *
 */

#pragma once

#include "AttributeDataIBs.h"
#include "MessageBuilder.h"
#include "MessageParser.h"
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace WriteRequestMessage {
enum class Tag : uint8_t
{
    kSuppressResponse    = 0,
    kTimedRequest        = 1,
    kWriteRequests       = 2,
    kMoreChunkedMessages = 3,
};

class Parser : public MessageParser
{
public:
#if CHIP_CONFIG_IM_PRETTY_PRINT
    CHIP_ERROR PrettyPrint() const;
#endif // CHIP_CONFIG_IM_PRETTY_PRINT
    /**
     *  @brief Get SuppressResponse boolean
     *
     *  @param [in] apSuppressResponse    A pointer to apSuppressResponse
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetSuppressResponse(bool * const apSuppressResponse) const;

    /**
     *  @brief Get TimedRequest boolean
     *
     *  @param [in] apTimedRequest    A pointer to apTimedRequest
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetTimedRequest(bool * const apTimedRequest) const;

    /**
     *  @brief Get a TLVReader for the AttributePathIBs. Next() must be called before accessing them.
     *
     *  @param [in] apAttributeDataIBs    A pointer to apAttributeDataIBs
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetWriteRequests(AttributeDataIBs::Parser * const apAttributeDataIBs) const;

    /**
     *  @brief Get MoreChunkedMessages boolean
     *
     *  @param [in] apMoreChunkedMessages    A pointer to apMoreChunkedMessages
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
     *  @brief This can be used to optionally signal to the server that no responses are to be sent back.
     *  @param [in] aSuppressResponse true if client need to signal suppress response
     *  @return A reference to *this
     */
    WriteRequestMessage::Builder & SuppressResponse(const bool aSuppressResponse);

    /**
     *  @brief flag action as part of a timed write transaction
     *  @param [in] aTimedRequest true if client need to signal this is a timed request
     *  @return A reference to *this
     */
    WriteRequestMessage::Builder & TimedRequest(const bool aTimedRequest);

    /**
     *  @brief Initialize a AttributeDataIBs::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributeDataIBs::Builder
     */
    AttributeDataIBs::Builder & CreateWriteRequests();

    /**
     *  @brief Set True if the set of AttributeDataIBs have to be sent across multiple packets in a single transaction
     *  @param [in] aMoreChunkedMessages  true if more chunked messaged is needed
     *  @return A reference to *this
     */
    WriteRequestMessage::Builder & MoreChunkedMessages(const bool aMoreChunkedMessages);

    AttributeDataIBs::Builder & GetWriteRequests() { return mWriteRequests; };

    /**
     *  @brief Mark the end of this WriteRequestMessage
     *
     *  @return The builder's final status.
     */
    CHIP_ERROR EndOfWriteRequestMessage();

private:
    AttributeDataIBs::Builder mWriteRequests;
};
} // namespace WriteRequestMessage
} // namespace app
} // namespace chip
