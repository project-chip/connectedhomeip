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
 *      This file defines AttributeDataVersionList parser and builder in CHIP interaction model
 *
 */

#pragma once

#include "AttributeDataElement.h"
#include "ListBuilder.h"
#include "ListParser.h"

#include <app/AppBuildConfig.h>
#include <app/util/basic-types.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace AttributeDataVersionList {
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
     *    The encoding rule has changed in IM encoding spec so this
     *    check is only "roughly" conformant now.
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR CheckSchemaValidity() const;
#endif

    /**
     *  @brief Check if this element is valid
     *
     *  @return A Boolean
     */
    bool IsElementValid(void);

    /**
     *  @brief Check if this element is NULL
     *
     *  @return A Boolean
     */
    bool IsNull(void);

    /**
     *  @brief Get a value for the DataVersion. Next() must be called before accessing them.
     *
     *  @param [in] apVersion    A pointer to apVersion
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetVersion(chip::DataVersion * const apVersion);
};

class Builder : public ListBuilder
{
public:
    /**
     *  @brief Add version in AttributeDataVersionList
     *
     *  @return A reference to AttributeDataVersionList::Builder
     */
    AttributeDataVersionList::Builder & AddVersion(const uint64_t aVersion);

    /**
     *  @brief Add Null in version list
     *
     *  @return A reference to *this
     */
    AttributeDataVersionList::Builder & AddNull(void);
    /**
     *  @brief Mark the end of this AttributeDataVersionList
     *
     *  @return A reference to *this
     */
    AttributeDataVersionList::Builder & EndOfAttributeDataVersionList();

private:
    AttributeDataElement::Builder mAttributeDataElementBuilder;
};
}; // namespace AttributeDataVersionList
}; // namespace app
}; // namespace chip
