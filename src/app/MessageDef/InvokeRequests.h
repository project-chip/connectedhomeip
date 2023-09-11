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
#include "CommandDataIB.h"

namespace chip {
namespace app {
namespace InvokeRequests {
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
     *  @brief Initialize a CommandDataIB::Builder for writing into the TLV stream
     *
     *  @return A reference to CommandDataIB::Builder
     */
    CommandDataIB::Builder & CreateCommandData();

    /**
     *  @return A reference to CommandDataIB::Builder
     */
    CommandDataIB::Builder & GetCommandData() { return mCommandData; };

    /**
     *  @brief Mark the end of this InvokeRequests
     *
     *  @return The builder's final status.
     */
    CHIP_ERROR EndOfInvokeRequests();

private:
    CommandDataIB::Builder mCommandData;
};
} // namespace InvokeRequests
} // namespace app
} // namespace chip
