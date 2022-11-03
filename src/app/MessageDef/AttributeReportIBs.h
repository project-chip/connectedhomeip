/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2016-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 *    @file
 *      This file defines AttributeReportIBs parser and builder in CHIP interaction model
 *
 */

#pragma once

#include "ArrayBuilder.h"
#include "ArrayParser.h"
#include "AttributeReportIB.h"

#include <app/AppBuildConfig.h>
#include <app/ConcreteAttributePath.h>
#include <app/MessageDef/StatusIB.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace AttributeReportIBs {
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

class Builder : public ArrayBuilder
{
public:
    /**
     *  @brief Initialize a AttributeReportIB::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributeReportIB::Builder
     */
    AttributeReportIB::Builder & CreateAttributeReport();

    AttributeReportIB::Builder & GetAttributeReport() { return mAttributeReport; }

    /**
     *  @brief Mark the end of this AttributeReportIBs
     *
     *  @return A reference to *this
     */
    AttributeReportIBs::Builder & EndOfAttributeReportIBs();

    /**
     * Encode an AttributeReportIB containing an AttributeStatus.
     */
    CHIP_ERROR EncodeAttributeStatus(const ConcreteReadAttributePath & aPath, const StatusIB & aStatus);

private:
    AttributeReportIB::Builder mAttributeReport;
};
} // namespace AttributeReportIBs
} // namespace app
} // namespace chip
