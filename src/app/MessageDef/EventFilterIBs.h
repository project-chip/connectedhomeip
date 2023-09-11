/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
     *  @return The builder's final status.
     */
    CHIP_ERROR EndOfEventFilters();

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
