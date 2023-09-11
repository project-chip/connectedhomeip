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
#include "AttributePathIB.h"

#include <app/AppBuildConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace AttributePathIBs {
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
     *  @brief Initialize a AttributePathIB::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributePathIB::Builder
     */
    AttributePathIB::Builder & CreatePath();

    /**
     *  @brief Mark the end of this AttributePathIB
     *
     *  @return The builder's final status.
     */
    CHIP_ERROR EndOfAttributePathIBs();

private:
    AttributePathIB::Builder mAttributePath;
};
} // namespace AttributePathIBs
} // namespace app
} // namespace chip
