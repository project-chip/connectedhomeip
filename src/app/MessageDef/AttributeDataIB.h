/**
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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

#include "AttributePathIB.h"
#include "StructBuilder.h"
#include "StructParser.h"

#include <app/AppConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace AttributeDataIB {
enum class Tag : uint8_t
{
    kDataVersion = 0,
    kPath        = 1,
    kData        = 2,
};

class Parser : public StructParser
{
public:
#if CHIP_CONFIG_IM_PRETTY_PRINT
    CHIP_ERROR PrettyPrint() const;
#endif // CHIP_CONFIG_IM_PRETTY_PRINT
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

    /**
     *  @brief Get a TLVReader for the AttributePathIB. Next() must be called before accessing them.
     *
     *  @param [in] apAttributePath    A pointer to apAttributePath
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a Path
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetPath(AttributePathIB::Parser * const apAttributePath) const;

    /**
     *  @brief Get a TLVReader for the Data. Next() must be called before accessing them.
     *
     *  @param [in] apReader    A pointer to apReader
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetData(TLV::TLVReader * const apReader) const;
};

class Builder : public StructBuilder
{
public:
    /**
     *  @brief Inject DataVersion into the TLV stream to indicate the numerical data version associated with
     *  the cluster that is referenced by the path.
     *
     *  @param [in] aDataVersion The boolean variable to indicate if AttributeDataIB has version
     *
     *  @return A reference to *this
     */
    AttributeDataIB::Builder & DataVersion(const chip::DataVersion aDataVersion);

    /**
     *  @brief Initialize a AttributePathIB::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributePathIB::Builder
     */
    AttributePathIB::Builder & CreatePath();

    /**
     *  @brief Mark the end of this AttributeDataIB
     *
     *  @return Our The builder's final status.
     */
    CHIP_ERROR EndOfAttributeDataIB();

private:
    AttributePathIB::Builder mPath;
};
} // namespace AttributeDataIB
} // namespace app
} // namespace chip
