/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "EventPathIB.h"
#include "StatusIB.h"
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
namespace EventStatusIB {
enum class Tag : uint8_t
{
    kPath        = 0,
    kErrorStatus = 1,
};

class Parser : public StructParser
{
public:
#if CHIP_CONFIG_IM_PRETTY_PRINT
    CHIP_ERROR PrettyPrint() const;
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

    /**
     *  @brief Get a TLVReader for the EventPathIB. Next() must be called before accessing them.
     *
     *  @param [in] apPath    A pointer to apPath
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a Path
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetPath(EventPathIB::Parser * const apPath) const;

    /**
     *  @brief Get a TLVReader for the StatusIB. Next() must be called before accessing them.
     *
     *  @param [in] apErrorStatus    A pointer to apErrorStatus
     *
     *  @return #CHIP_NO_ERROR on success
     *          # CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a structure
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetErrorStatus(StatusIB::Parser * const apErrorStatus) const;
};

class Builder : public StructBuilder
{
public:
    /**
     *  @brief Initialize a EventPathIB::Builder for writing into the TLV stream
     *
     *  @return A reference to EventPathIB::Builder
     */
    EventPathIB::Builder & CreatePath();

    /**
     *  @brief Initialize a StatusIB::Builder for writing into the TLV stream
     *
     *  @return A reference to StatusIB::Builder
     */
    StatusIB::Builder & CreateErrorStatus();

    /**
     *  @brief Mark the end of this EventStatusIB
     *
     *  @return The builder's final status.
     */
    CHIP_ERROR EndOfEventStatusIB();

private:
    EventPathIB::Builder mPath;
    StatusIB::Builder mErrorStatus;
};
} // namespace EventStatusIB
} // namespace app
} // namespace chip
