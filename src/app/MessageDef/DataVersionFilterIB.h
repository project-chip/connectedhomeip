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

#include "ClusterPathIB.h"
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
namespace DataVersionFilterIB {
enum class Tag : uint8_t
{
    kPath        = 0,
    kDataVersion = 1,
};

class Parser : public StructParser
{
public:
#if CHIP_CONFIG_IM_PRETTY_PRINT
    CHIP_ERROR PrettyPrint() const;
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

    /**
     *  @brief Get a TLVReader for the AttributePathIB. Next() must be called before accessing them.
     *
     *  @param [in] apPath    A pointer to apPath
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a Path
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetPath(ClusterPathIB::Parser * const apPath) const;

    /**
     *  @brief Get the DataVersion.
     *
     *  @param [in] apVersion    A pointer to apVersion
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetDataVersion(chip::DataVersion * const apVersion) const;
};

class Builder : public StructBuilder
{
public:
    /**
     *  @brief Initialize a ClusterPathIB::Builder for writing into the TLV stream
     *
     *  @return A reference to ClusterPathIB::Builder
     */
    ClusterPathIB::Builder & CreatePath();

    /**
     *  @brief Inject DataVersion into the TLV stream to indicate the numerical data version associated with
     *  the cluster that is referenced by the path.
     *
     *  @param [in] aDataVersion The unsigned integer variable
     *
     *  @return A reference to *this
     */
    DataVersionFilterIB::Builder & DataVersion(const chip::DataVersion aDataVersion);

    /**
     *  @brief Mark the end of this DataVersionFilterIB
     *
     *  @return A reference to *this
     */
    DataVersionFilterIB::Builder & EndOfDataVersionFilterIB();

private:
    ClusterPathIB::Builder mPath;
};
}; // namespace DataVersionFilterIB
}; // namespace app
}; // namespace chip
