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
 *      This file defines EventFilters parser and builder in CHIP interaction model
 *
 */

#pragma once

#include <app/AppBuildConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include "EventFilterIB.h"
#include "ListBuilder.h"
#include "ListParser.h"

namespace chip {
namespace app {
namespace EventFilters {
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
};

class Builder : public ListBuilder
{
public:
    /**
     *  @brief Initialize a EventFilterIB::Builder for writing into the TLV stream
     *
     *  @return A reference to EventFilterIB::Builder
     */
    EventFilterIB::Builder & CreateEventFilter();

    /**
     *  @return A reference to EventFilterIB::Builder
     */
    EventFilterIB::Builder & GetEventFilter() { return mEventFilter; };

    /**
     *  @brief Mark the end of this EventFilters
     *
     *  @return A reference to *this
     */
    EventFilters::Builder & EndOfEventFilters();

private:
    EventFilterIB::Builder mEventFilter;
};
}; // namespace EventFilters
}; // namespace app
}; // namespace chip
