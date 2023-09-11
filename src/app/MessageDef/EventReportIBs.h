/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 *    @file
 *      This file defines EventReportIBs parser and builder in CHIP interaction model
 *
 */

#pragma once

#include "ArrayBuilder.h"
#include "ArrayParser.h"
#include "EventReportIB.h"

#include <app/AppBuildConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace EventReportIBs {
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
     *  @brief Initialize a EventReportIB::Builder for writing into the TLV stream
     *
     *  @return A reference to EventReportIB::Builder
     */
    EventReportIB::Builder & CreateEventReport();

    /**
     *  @brief Mark the end of this EventReportIBs
     *
     *  @return The builder's final status.
     */
    CHIP_ERROR EndOfEventReports();

private:
    EventReportIB::Builder mEventReport;
};
} // namespace EventReportIBs
} // namespace app
} // namespace chip
