/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
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
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace AttributeReportIBs {
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
     *  @brief Initialize a AttributeReportIB::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributeReportIB::Builder
     */
    AttributeReportIB::Builder & CreateAttributeReport();

    AttributeReportIB::Builder & GetAttributeReport() { return mAttributeReport; }

    /**
     *  @brief Mark the end of this AttributeReportIBs
     *
     *  @return The builder's final status.
     */
    CHIP_ERROR EndOfAttributeReportIBs();

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
