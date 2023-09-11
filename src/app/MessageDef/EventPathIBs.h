/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "ArrayBuilder.h"
#include "ArrayParser.h"
#include "EventPathIB.h"
#include "EventPathIBs.h"

#include <app/AppBuildConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace EventPathIBs {
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
     *  @brief Initialize a EventPathIB::Builder for writing into the TLV stream
     *
     *  @return A reference to EventPathIB::Builder
     */
    EventPathIB::Builder & CreatePath();

    /**
     *  @brief Mark the end of this EventPathIBs
     *
     *  @return The builder's final status.
     */
    CHIP_ERROR EndOfEventPaths();

private:
    EventPathIB::Builder mEventPath;
};
} // namespace EventPathIBs
} // namespace app
} // namespace chip
