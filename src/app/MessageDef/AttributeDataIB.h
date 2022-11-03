/*
 * SPDX-FileCopyrightText: (c) 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2016-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "AttributePathIB.h"
#include "StructBuilder.h"
#include "StructParser.h"

#include <app/AppBuildConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
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
     *  @return A reference to *this
     */
    AttributeDataIB::Builder & EndOfAttributeDataIB();

private:
    AttributePathIB::Builder mPath;
};
} // namespace AttributeDataIB
} // namespace app
} // namespace chip
