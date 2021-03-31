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
 *      This file defines StatusList parser and builder in CHIP interaction model
 *
 */

#pragma once

#include "StatusElement.h"
#include "ListBuilder.h"
#include "ListParser.h"

#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <util/basic-types.h>

namespace chip {
namespace app {
namespace StatusList {
class Parser : public ListParser
{
public:
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
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
#endif
};

class Builder : public ListBuilder
{
public:
    /**
     *  @brief Initialize a StatusElement::Builder for writing into the TLV stream
     *
     *  @return A reference to StatusElement::Builder
     */
    StatusElement::Builder & CreateStatusElementBuilder();

    /**
     *  @brief Mark the end of this StatusList
     *
     *  @return A reference to *this
     */
    StatusList::Builder & EndOfStatusList();

private:
    StatusElement::Builder mStatusElementBuilder;
};
};
};
};
