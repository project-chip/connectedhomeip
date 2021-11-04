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

#include "AttributeDataVersionList.h"
#include "AttributeDatas.h"
#include "Builder.h"
#include "Parser.h"
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace WriteRequestMessage {
enum
{
    kCsTag_SuppressResponse         = 0,
    kCsTag_AttributeDatas           = 1,
    kCsTag_AttributeDataVersionList = 2,
    kCsTag_MoreChunkedMessages      = 3,
};

class Parser : public chip::app::Parser
{
public:
    /**
     *  @brief Initialize the parser object with TLVReader
     *
     *  @param [in] aReader A pointer to a TLVReader, which should point to the beginning of this request
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    /**
     *  @brief Roughly verify the message is correctly formed
     *   1) all mandatory tags are present
     *   2) all elements have expected data type
     *   3) any tag can only appear once
     *   4) At the top level of the structure, unknown tags are ignored for forward compatibility
     *  @note The main use of this function is to print out what we're
     *    receiving during protocol development and debugging.
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR CheckSchemaValidity() const;

    /**
     *  @brief Get GetSuppressResponse Next() must be called before accessing them.
     *
     *  @param [in] apSuppressResponse    A pointer to apSuppressResponse
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetSuppressResponse(bool * const apSuppressResponse) const;

    /**
     *  @brief Get a TLVReader for the AttributePaths. Next() must be called before accessing them.
     *
     *  @param [in] apAttributeDatas    A pointer to apAttributeDatas
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetAttributeReports(AttributeDatas::Parser * const apAttributeDatas) const;

    /**
     *  @brief Get a TLVReader for the AttributeDataVersionList. Next() must be called before accessing them.
     *
     *  @param [in] apAttributeDataVersionList    A pointer to apAttributeDataVersionList
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetAttributeDataVersionList(AttributeDataVersionList::Parser * const apAttributeDataVersionList) const;

    /**
     *  @brief Get MoreChunkedMessages message. Next() must be called before accessing them.
     *
     *  @param [in] apMoreChunkedMessages    A pointer to apMoreChunkedMessages
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
     *  @brief Initialize a WriteRequestMessage::Builder for writing into a TLV stream
     *
     *  @param [in] apWriter    A pointer to TLVWriter
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    /**
     *  @brief This can be used to optionally signal to the server that no responses are to be sent back.
     *  @param [in] aSuppressResponse true if client need to signal suppress response
     *  @return A reference to *this
     */
    WriteRequestMessage::Builder & SuppressResponse(const bool aSuppressResponse);

    /**
     *  @brief Initialize a AttributeDatas::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributeDatas::Builder
     */
    AttributeDatas::Builder & CreateAttributeDatasBuilder();

    /**
     *  @brief Initialize a AttributeDataVersionList::Builder for writing into the TLV stream
     *
     *  @return A reference to EventPaths::Builder
     */
    AttributeDataVersionList::Builder & CreateAttributeDataVersionListBuilder();

    /**
     *  @brief Set True if the set of AttributeDataIBs have to be sent across multiple packets in a single transaction
     *  @param [in] aMoreChunkedMessages  true if more chunked messaged is needed
     *  @return A reference to *this
     */
    WriteRequestMessage::Builder & MoreChunkedMessages(const bool aMoreChunkedMessages);

    AttributeDatas::Builder & GetAttributeReportsBuilder();

    /**
     *  @brief Mark the end of this WriteRequestMessage
     *
     *  @return A reference to *this
     */
    WriteRequestMessage::Builder & EndOfWriteRequestMessage();

private:
    AttributeDatas::Builder mAttributeDatasBuilder;
    AttributeDataVersionList::Builder mAttributeDataVersionListBuilder;
};
}; // namespace WriteRequestMessage
}; // namespace app
}; // namespace chip
