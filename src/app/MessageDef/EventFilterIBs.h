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

#pragma once

#include <app/AppBuildConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include "ArrayBuilder.h"
#include "ArrayParser.h"
#include "EventFilterIB.h"

namespace chip {
namespace app {
namespace EventFilterIBs {
class Parser : public ArrayParser
{
public:
#if CHIP_CONFIG_IM_PRETTY_PRINT
    CHIP_ERROR PrettyPrint() const;
#endif // CHIP_CONFIG_IM_PRETTY_PRINT
};

class Builder : public ArrayBuilder
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
     *  @brief Mark the end of this EventFilterIBs
     *
     *  @return A reference to *this
     */
    EventFilterIBs::Builder & EndOfEventFilters();

    /**
     *  @brief Generate single event filter
     *
     */
    CHIP_ERROR GenerateEventFilter(EventNumber aEventNumber);

private:
    EventFilterIB::Builder mEventFilter;
};
}; // namespace EventFilterIBs
}; // namespace app
}; // namespace chip
