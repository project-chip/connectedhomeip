/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <app/AppConfig.h>
#include <app/DataVersionFilter.h>
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
     * Add a DataVersionFilter to the list.  This is a convenience method
     * that will handle calling CreateDataVersionFilter() and then using the
     * result to encode the provided DataVersionFilter.
     *
     * The passed-in DataVersionFilter is assumed to pass the
     * IsValidDataVersionFilter() test.
     */
    CHIP_ERROR EncodeDataVersionFilterIB(const DataVersionFilter & aFilter);

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
