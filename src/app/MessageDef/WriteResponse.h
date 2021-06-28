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
 *      This file defines WriteResponse parser and builder in CHIP interaction model
 *
 */

#pragma once

#include "AttributeStatusList.h"
#include "Builder.h"

#include "Parser.h"
#include <app/util/basic-types.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace WriteResponse {
enum
{
    kCsTag_AttributeStatusList = 0,
};

class Parser : public chip::app::Parser
{
public:
    /**
     *  @brief Initialize the parser object with TLVReader
     *
     *  @param [in] aReader A pointer to a TLVReader, which should point to the beginning of this response
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
     *  @brief Get a TLVReader for the AttributeStatusList. Next() must be called before accessing them.
     *
     *  @param [in] apAttributeStatusList    A pointer to apAttributeStatusList
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetAttributeStatusList(AttributeStatusList::Parser * const apAttributeStatusList) const;
};

class Builder : public chip::app::Builder
{
public:
    /**
     *  @brief Initialize a WriteResponse::Builder for writing into a TLV stream
     *
     *  @param [in] apWriter    A pointer to TLVWriter
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    /**
     *  @brief Initialize a AttributeStatusList::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributeStatusList::Builder
     */
    AttributeStatusList::Builder & CreateAttributeStatusListBuilder();

    /**
     *  @brief Get reference to AttributeStatusList::Builder
     *
     *  @return A reference to AttributeStatusList::Builder
     */
    AttributeStatusList::Builder & GetAttributeStatusListBuilder();

    /**
     *  @brief Mark the end of this WriteResponse
     *
     *  @return A reference to *this
     */
    WriteResponse::Builder & EndOfWriteResponse();

private:
    AttributeStatusList::Builder mAttributeStatusListBuilder;
};
}; // namespace WriteResponse
}; // namespace app
}; // namespace chip
