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
 *      This file defines CommandDataElement parser and builder in CHIP interaction model
 *
 */

#pragma once

#include "Builder.h"
#include "CommandPath.h"

#include "Parser.h"
#include "StatusElement.h"

#include <app/AppBuildConfig.h>
#include <app/util/basic-types.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace CommandDataElement {
enum
{
    kCsTag_CommandPath   = 0,
    kCsTag_Data          = 1,
    kCsTag_StatusElement = 2,
};

class Parser : public chip::app::Parser
{
public:
    /**
     *  @brief Initialize the parser object with TLVReader
     *
     *  @param [in] aReader A pointer to a TLVReader, which should point to the beginning of this CommandDataElement
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
     *  @brief Get a TLVReader for the CommandPath. Next() must be called before accessing them.
     *
     *  @param [in] apCommandPath    A pointer to apCommandPath
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a Path
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetCommandPath(CommandPath::Parser * const apCommandPath) const;

    /**
     *  @brief Get a TLVReader for the Data. Next() must be called before accessing them.
     *
     *  @param [in] apReader    A pointer to apReader
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetData(chip::TLV::TLVReader * const apReader) const;

    /**
     *  @brief Get a TLVReader for the StatusElement. Next() must be called before accessing them.
     *
     *  @param [in] apStatusElement    A pointer to apStatusElement
     *
     *  @return #CHIP_NO_ERROR on success
     *          # CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a structure
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetStatusElement(StatusElement::Parser * const apStatusElement) const;

protected:
    // A recursively callable function to parse a data element and pretty-print it.
    CHIP_ERROR ParseData(chip::TLV::TLVReader & aReader, int aDepth) const;
};

class Builder : public chip::app::Builder
{
public:
    /**
     *  @brief Initialize a AttributeDataList::Builder for writing into a TLV stream
     *
     *  @param [in] apWriter    A pointer to TLVWriter
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    /**
     *  @brief Initialize a CommandPath::Builder for writing into the TLV stream
     *
     *  @return A reference to CommandPath::Builder
     */
    CommandPath::Builder & CreateCommandPathBuilder();

    /**
     *  @brief Initialize a StatusElement::Builder for writing into the TLV stream
     *
     *  @return A reference to StatusElement::Builder
     */
    StatusElement::Builder & CreateStatusElementBuilder();

    /**
     *  @brief Mark the end of this CommandDataElement
     *
     *  @return A reference to *this
     */
    CommandDataElement::Builder & EndOfCommandDataElement();

private:
    CommandPath::Builder mCommandPathBuilder;
    StatusElement::Builder mStatusElementBuilder;
};
}; // namespace CommandDataElement
}; // namespace app
}; // namespace chip
