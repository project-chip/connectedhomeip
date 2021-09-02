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
 *      This file defines ReadRequest parser and builder in CHIP interaction model
 *
 */

#pragma once

#include "AttributeDataVersionList.h"
#include "AttributePathList.h"
#include "Builder.h"
#include "EventPathList.h"

#include "Parser.h"

#include <app/AppBuildConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace ReadRequest {
enum
{
    kCsTag_AttributePathList        = 0,
    kCsTag_EventPathList            = 1,
    kCsTag_AttributeDataVersionList = 2,
    kCsTag_EventNumber              = 3,
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
     *  @brief Get a TLVReader for the AttributePathList. Next() must be called before accessing them.
     *
     *  @param [in] apAttributePathList    A pointer to an attribute path list parser.
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetAttributePathList(AttributePathList::Parser * const apAttributePathList) const;

    /**
     *  @brief Get a TLVReader for the EventPathList. Next() must be called before accessing them.
     *
     *  @param [in] apEventPathList    A pointer to apEventPathList
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEventPathList(EventPathList::Parser * const apEventPathList) const;

    /**
     *  @brief Get a parser for the AttributeDataVersionList. Next() must be called before accessing them.
     *
     *  @param [in] apAttributeDataVersionList    A pointer to apAttributeDataVersionList
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetAttributeDataVersionList(AttributeDataVersionList::Parser * const apAttributeDataVersionList) const;

    /**
     *  @brief Get Event Number. Next() must be called before accessing them.
     *
     *  @param [in] apEventNumber    A pointer to apEventNumber
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEventNumber(uint64_t * const apEventNumber) const;
};

class Builder : public chip::app::Builder
{
public:
    /**
     *  @brief Initialize a ReadRequest::Builder for writing into a TLV stream
     *
     *  @param [in] apWriter    A pointer to TLVWriter
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    /**
     *  @brief Initialize a AttributePathList::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributePathList::Builder
     */
    AttributePathList::Builder & CreateAttributePathListBuilder();

    /**
     *  @brief Initialize a EventPathList::Builder for writing into the TLV stream
     *
     *  @return A reference to EventPathList::Builder
     */
    EventPathList::Builder & CreateEventPathListBuilder();

    /**
     *  @brief Initialize a AttributeDataVersionList::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributeDataVersionList::Builder
     */
    AttributeDataVersionList::Builder & CreateAttributeDataVersionListBuilder();

    /**
     *  @brief An initiator can optionally specify an EventNumber it has already to limit the
     *  set of retrieved events on the server for optimization purposes.
     *  @param [in] aEventNumber The event number
     *  @return A reference to *this
     */
    ReadRequest::Builder & EventNumber(const uint64_t aEventNumber);
    /**
     *  @brief Mark the end of this ReadRequest
     *
     *  @return A reference to *this
     */
    ReadRequest::Builder & EndOfReadRequest();

private:
    AttributePathList::Builder mAttributePathListBuilder;
    EventPathList::Builder mEventPathListBuilder;
    AttributeDataVersionList::Builder mAttributeDataVersionListBuilder;
};
}; // namespace ReadRequest
}; // namespace app
}; // namespace chip
