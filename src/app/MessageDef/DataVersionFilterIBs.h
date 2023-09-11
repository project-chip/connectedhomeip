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
#include "DataVersionFilterIB.h"

namespace chip {
namespace app {
namespace DataVersionFilterIBs {
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
     *  @brief Initialize a DataVersionFilterIB::Builder for writing into the TLV stream
     *
     *  @return A reference to DataVersionFilterIB::Builder
     */
    DataVersionFilterIB::Builder & CreateDataVersionFilter();

    /**
     *  @return A reference to DataVersionFilterIB::Builder
     */
    DataVersionFilterIB::Builder & GetDataVersionFilter() { return mDataVersionFilter; };

    /**
     *  @brief Mark the end of this DataVersionFilterIBs
     *
     *  @return The builder's final status.
     */
    CHIP_ERROR EndOfDataVersionFilterIBs();

private:
    DataVersionFilterIB::Builder mDataVersionFilter;
};
} // namespace DataVersionFilterIBs
} // namespace app
} // namespace chip
