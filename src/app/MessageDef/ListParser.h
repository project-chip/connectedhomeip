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
 *      This file defines List parser in CHIP interaction model
 *
 */

#pragma once

#ifndef _CHIP_INTERACTION_MODEL_MESSAGE_DEF_LIST_PARSER_H
#define _CHIP_INTERACTION_MODEL_MESSAGE_DEF_LIST_PARSER_H

#include "Builder.h"
#include "Parser.h"
#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <util/basic-types.h>

namespace chip {
namespace app {
class ListParser : public chip::app::Parser
{
protected:
    ListParser();

public:
    /**
     *  @brief Initialize the parser object with TLVReader
     *
     *  @param [in] aReader A pointer to a TLVReader, which should be on the element of the array element
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    /**
     *  @brief Initialize the parser object with TLVReader if context tag exists
     *
     *  @param [in] aReader A pointer to a TLVReader, which should be on the element of the array element
     *  @param [in] aContextTagToFind A context tag it tries to find
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR InitIfPresent(const chip::TLV::TLVReader & aReader, const uint8_t aContextTagToFind);

    /**
     *  @brief Iterate to next element
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Next();
};

}; // namespace app
}; // namespace chip

#endif // _CHIP_INTERACTION_MODEL_MESSAGE_DEF_LIST_PARSER_H
