/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 *    @file
 *      This file defines AttributeDataIBs parser and builder in CHIP interaction model
 *
 */

#pragma once

#include "ArrayBuilder.h"
#include "ArrayParser.h"
#include "AttributeDataIB.h"

#include <app/AppBuildConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace AttributeDataIBs {
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
     *  @brief Initialize a AttributeDataIB::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributeDataIB::Builder
     */
    AttributeDataIB::Builder & CreateAttributeDataIBBuilder();

    AttributeDataIB::Builder & GetAttributeDataIBBuilder();

    /**
     *  @brief Mark the end of this AttributeDataIBs
     *
     *  @return The builder's final status.
     */
    CHIP_ERROR EndOfAttributeDataIBs();

private:
    AttributeDataIB::Builder mAttributeDataIBBuilder;
};
}; // namespace AttributeDataIBs

}; // namespace app
}; // namespace chip
