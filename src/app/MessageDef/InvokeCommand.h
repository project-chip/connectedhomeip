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
 *      This file defines InvokeCommand parser and builder in CHIP interaction model
 *
 */

#pragma once

#include <app/AppBuildConfig.h>
#include <app/util/basic-types.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include "Builder.h"
#include "CommandDataElement.h"
#include "CommandList.h"
#include "Parser.h"

namespace chip {
namespace app {
namespace InvokeCommand {
enum
{
    kCsTag_CommandList = 0,
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
     *  @brief Get a parser for a CommandList.
     *
     *  @param [in] apCommandList    A pointer to the command list parser.
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetCommandList(CommandList::Parser * const apCommandList) const;
};

class Builder : public chip::app::Builder
{
public:
    /**
     *  @brief Initialize a InvokeCommand::Builder for writing into a TLV stream
     *
     *  @param [in] apWriter    A pointer to TLVWriter
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    /**
     *  @brief Initialize a CommandList::Builder for writing into the TLV stream
     *
     *  @return A reference to CommandList::Builder
     */
    CommandList::Builder & CreateCommandListBuilder();

    /**
     *  @brief Get reference to CommandList::Builder
     *
     *  @return A reference to CommandList::Builder
     */
    CommandList::Builder & GetCommandListBuilder();

    /**
     *  @brief Mark the end of this InvokeCommand
     *
     *  @return A reference to *this
     */
    InvokeCommand::Builder & EndOfInvokeCommand();

private:
    CommandList::Builder mCommandListBuilder;
};
}; // namespace InvokeCommand
}; // namespace app
}; // namespace chip
