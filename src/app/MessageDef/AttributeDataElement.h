/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
/**
 *    @file
 *      This file defines AttributeDataElement parser and builder in CHIP interaction model
 *
 */

#pragma once

#include "AttributePath.h"
#include "Builder.h"
#include "Parser.h"

#include <app/AppBuildConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace AttributeDataElement {
enum
{
    kCsTag_AttributePath       = 0,
    kCsTag_DataVersion         = 1,
    kCsTag_Data                = 2,
    kCsTag_Status              = 3,
    kCsTag_MoreClusterDataFlag = 4,
};

class Parser : public chip::app::Parser
{
public:
    /**
     *  @brief Initialize the parser object with TLVReader
     *
     *  @param [in] aReader A pointer to a TLVReader, which should point to the beginning of this AttributeDataElement
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

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
     *  @brief Get a TLVReader for the AttributePath. Next() must be called before accessing them.
     *
     *  @param [in] apAttributePath    A pointer to apAttributePath
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a Path
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetAttributePath(AttributePath::Parser * const apAttributePath) const;

    /**
     *  @brief Get a TLVReader for the DataVersion. Next() must be called before accessing them.
     *
     *  @param [in] apVersion    A pointer to apVersion
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetDataVersion(chip::DataVersion * const apVersion) const;

    /**
     *  @brief Get a TLVReader for the Data. Next() must be called before accessing them.
     *
     *  @param [in] apReader    A pointer to apReader
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetData(chip::TLV::TLVReader * const apReader) const;

    /**
     *  @brief Get the IM status code.
     *
     *  @param [out] apStatus A pointer to result
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is not such element
     */
    CHIP_ERROR GetStatus(uint16_t * const apStatus) const;

    /**
     *  @brief Check whether it need more cluster data Next() must be called before accessing them.
     *
     *  @param [in] apMoreClusterDataFlag    A pointer to apMoreClusterDataFlag
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetMoreClusterDataFlag(bool * const apMoreClusterDataFlag) const;

protected:
    // A recursively callable function to parse a data element and pretty-print it.
    CHIP_ERROR ParseData(chip::TLV::TLVReader & aReader, int aDepth) const;
};

class Builder : public chip::app::Builder
{
public:
    /**
     *  @brief Initialize a AttributeDataElement::Builder for writing into a TLV stream
     *
     *  @param [in] apWriter    A pointer to TLVWriter
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    /**
     *  @brief Initialize a AttributePath::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributePath::Builder
     */
    AttributePath::Builder & CreateAttributePathBuilder();

    /**
     *  @brief Inject DataVersion into the TLV stream to indicate the numerical data version associated with
     *  the cluster that is referenced by the path.
     *
     *  @param [in] aDataVersion The boolean variable to indicate if AttributeDataElement has version
     *
     *  @return A reference to *this
     */
    AttributeDataElement::Builder & DataVersion(const chip::DataVersion aDataVersion);

    /**
     *  @brief Inject aMoreClusterData into the TLV stream to indicate whether there is more cluster data.
     *  This is present when there is more than one AttributeDataElement as part of a logical Changeset,
     *  and the entire set needs to be applied ‘atomically’ on the receiver.
     *
     *  @param [in] aMoreClusterData The boolean variable to indicate if more cluster data is needed.
     *
     *  @return A reference to *this
     */
    AttributeDataElement::Builder & MoreClusterData(const bool aMoreClusterData);

    /**
     *  @brief Mark the end of this AttributeDataElement
     *
     *  @return A reference to *this
     */
    AttributeDataElement::Builder & EndOfAttributeDataElement();

private:
    AttributePath::Builder mAttributePathBuilder;
};
}; // namespace AttributeDataElement

}; // namespace app
}; // namespace chip
