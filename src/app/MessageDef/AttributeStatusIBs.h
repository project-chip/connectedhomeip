/*
 * SPDX-FileCopyrightText: (c) 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2016-2017 Nest Labs, Inc.
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
#include <lib/core/CHIPTLV.h>
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
     *  @return A reference to *this
     */
    AttributeStatusIBs::Builder & EndOfAttributeStatuses();

private:
    AttributeStatusIB::Builder mAttributeStatus;
};

class Parser : public ArrayParser
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
} // namespace AttributeStatusIBs
} // namespace app
} // namespace chip
