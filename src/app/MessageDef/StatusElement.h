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
 *      This file defines StatusElement parser and builder in CHIP interaction model
 *
 */

#pragma once

#include "ListBuilder.h"
#include "ListParser.h"

#include <app/AppBuildConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/secure_channel/Constants.h>

namespace chip {
namespace app {
namespace StatusElement {
enum
{
    kCsTag_GeneralCode  = 1,
    kCsTag_ProtocolId   = 2,
    kCsTag_ProtocolCode = 3,
    kCsTag_ClusterId    = 4
};

class Parser : public ListParser
{
public:
    /**
     *  @brief Initialize the parser object with TLVReader
     *
     *  @param [in] aReader A pointer to a TLVReader, which should point to the beginning of this StatusElement
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
    `* Read the GeneralCode, ProtocolId, ProtocolCode, ClusterId
     *
     * @param[out]   apGeneralCode     Pointer to the storage for the GeneralCode
     * @param[out]   apProtocolId     Pointer to the storage for the ProtocolId
     * @param[out]   apProtocolCode   Pointer to the storage for the ProtocolCode
     *
     * @return       CHIP_ERROR codes returned by chip::TLV objects. CHIP_END_OF_TLV if either
     *               element is missing. CHIP_ERROR_WRONG_TLV_TYPE if the elements are of the wrong
     *               type.
     */
    CHIP_ERROR DecodeStatusElement(Protocols::SecureChannel::GeneralStatusCode * apGeneralCode, uint32_t * apProtocolId,
                                   uint16_t * apProtocolCode) const;
};

class Builder : public ListBuilder
{
public:
    /**
     *  @brief Initialize a StatusElement::Builder for writing into a TLV stream
     *
     *  @param [in] apWriter    A pointer to TLVWriter
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    /**
     * Init the StatusElement container with an particular context tag.
     * Required to implement arrays of arrays, and to test ListBuilder.
     *
     * @param[in]   apWriter    Pointer to the TLVWriter that is encoding the message.
     * @param[in]   aContextTagToUse    A contextTag to use.
     *
     * @return                  CHIP_ERROR codes returned by chip::TLV objects.
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse);

    /**
    `* Read the GeneralCode, ProtocolId, ProtocolCode, ClusterId
     *
     * @param[in]   aGeneralCode    General status code
     * @param[in]   aProtocolId     A protocol ID (32-bit integer composed of a 16-bit vendor id and 16-bit Scoped id)
     * @param[in]   aProtocolCode   16-bit protocol-specific error code
     *
     * @return       CHIP_ERROR codes returned by chip::TLV objects. CHIP_END_OF_TLV if either
     *               element is missing. CHIP_ERROR_WRONG_TLV_TYPE if the elements are of the wrong
     *               type.
     */
    StatusElement::Builder & EncodeStatusElement(const Protocols::SecureChannel::GeneralStatusCode aGeneralCode,
                                                 const uint32_t aProtocolId, const uint16_t aProtocolCode);

    /**
     *  @brief Mark the end of this StatusElement
     *
     *  @return A reference to *this
     */
    StatusElement::Builder & EndOfStatusElement();
};
}; // namespace StatusElement

}; // namespace app
}; // namespace chip
