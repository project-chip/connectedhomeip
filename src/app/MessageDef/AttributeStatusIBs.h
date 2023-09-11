/**
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "ArrayBuilder.h"
#include "ArrayParser.h"
#include "AttributeStatusIB.h"

#include <app/AppBuildConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace AttributeStatusIBs {
class Builder : public ArrayBuilder
{
public:
    /**
     *  @brief Initialize a AttributeStatusIB::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributeStatusIB::Builder
     */
    AttributeStatusIB::Builder & CreateAttributeStatus();

    /**
     *  @brief Mark the end of this AttributeStatusIBs
     *
     *  @return The builder's final status.
     */
    CHIP_ERROR EndOfAttributeStatuses();

private:
    AttributeStatusIB::Builder mAttributeStatus;
};

class Parser : public ArrayParser
{
public:
#if CHIP_CONFIG_IM_PRETTY_PRINT
    CHIP_ERROR PrettyPrint() const;
#endif // CHIP_CONFIG_IM_PRETTY_PRINT
};
} // namespace AttributeStatusIBs
} // namespace app
} // namespace chip
