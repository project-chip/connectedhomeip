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
 *      This file defines AttributePathList parser and builder in CHIP interaction model
 *
 */

#pragma once

#ifndef _CHIP_INTERACTION_MODEL_MESSAGE_DEF_ATTRIBUTE_PATH_LIST_H
#define _CHIP_INTERACTION_MODEL_MESSAGE_DEF_ATTRIBUTE_PATH_LIST_H

#include "AttributePath.h"
#include "ListBuilder.h"
#include "ListParser.h"

#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <util/basic-types.h>

namespace chip {
namespace app {
namespace AttributePathList {
class Parser : public ListParser
{
public:
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
};

class Builder : public ListBuilder
{
public:
    /**
     *  @brief Initialize a AttributePath::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributePath::Builder
     */
    AttributePath::Builder & CreateAttributePathBuilder();

    /**
     *  @brief Mark the end of this AttributePath
     *
     *  @return A reference to *this
     */
    AttributePathList::Builder & EndOfAttributePathList();

private:
    AttributePath::Builder mAttributePathBuilder;
};
}; // namespace AttributePathList
}; // namespace app
}; // namespace chip

#endif // _CHIP_INTERACTION_MODEL_MESSAGE_DEF_ATTRIBUTE_PATH_LIST_H
