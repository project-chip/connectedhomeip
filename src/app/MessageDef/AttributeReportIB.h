/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "AttributeDataIB.h"
#include "AttributeStatusIB.h"
#include "StructBuilder.h"
#include "StructParser.h"

#include <app/AppBuildConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace AttributeReportIB {
enum class Tag : uint8_t
{
    kAttributeStatus = 0,
    kAttributeData   = 1,
};

class Parser : public StructParser
{
public:
#if CHIP_CONFIG_IM_PRETTY_PRINT
    CHIP_ERROR PrettyPrint() const;
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

    /**
     *  @brief Get a TLVReader for the StatusIB. Next() must be called before accessing them.
     *
     *  @param [in] apAttributeStatus    A pointer to apAttributeStatus
     *
     *  @return #CHIP_NO_ERROR on success
     *          # CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a structure
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetAttributeStatus(AttributeStatusIB::Parser * const apAttributeStatus) const;

    /**
     *  @brief Get a TLVReader for the AttributeDataIB. Next() must be called before accessing them.
     *
     *  @param [in] apAttributeData    A pointer to apAttributeData
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a AttributeData
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetAttributeData(AttributeDataIB::Parser * const apAttributeData) const;
};

class Builder : public StructBuilder
{
public:
    /**
     *  @brief Initialize a AttributeDataIB::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributeDataIB::Builder
     */
    AttributeDataIB::Builder & CreateAttributeData();

    AttributeDataIB::Builder & GetAttributeData() { return mAttributeData; }
    /**
     *  @brief Initialize a StatusIB::Builder for writing into the TLV stream
     *
     *  @return A reference to StatusIB::Builder
     */
    AttributeStatusIB::Builder & CreateAttributeStatus();

    /**
     *  @brief Mark the end of this AttributeReportIB
     *
     *  @return The builder's final status.
     */
    CHIP_ERROR EndOfAttributeReportIB();

private:
    AttributeStatusIB::Builder mAttributeStatus;
    AttributeDataIB::Builder mAttributeData;
};
} // namespace AttributeReportIB
} // namespace app
} // namespace chip
