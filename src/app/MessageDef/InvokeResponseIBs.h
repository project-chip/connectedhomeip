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
#include "InvokeResponseIB.h"

namespace chip {
namespace app {
namespace InvokeResponseIBs {
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
     *  @brief Initialize a InvokeResponseIB::Builder for writing into the TLV stream
     *
     *  @return A reference to InvokeResponseIB::Builder
     */
    InvokeResponseIB::Builder & CreateInvokeResponse();

    /**
     *  @return A reference to InvokeResponseIB::Builder
     */
    InvokeResponseIB::Builder & GetInvokeResponse() { return mInvokeResponse; };

    /**
     *  @brief Mark the end of this InvokeResponseIBs
     *
     *  @return The builder's final status.
     */
    CHIP_ERROR EndOfInvokeResponses();

private:
    InvokeResponseIB::Builder mInvokeResponse;
};
} // namespace InvokeResponseIBs
} // namespace app
} // namespace chip
