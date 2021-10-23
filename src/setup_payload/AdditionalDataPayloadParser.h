/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file describes a AdditionalData Payload parser based on the
 *      CHIP specification.
 */

#pragma once

#include "AdditionalDataPayload.h"

#include <lib/core/CHIPError.h>
#include <stdint.h>

namespace chip {

/**
 * @class AdditionalDataPayloadParser
 * A class that can be used to convert a HEX encoded payload to a AdditionalDataPayload object
 * */
class AdditionalDataPayloadParser
{
private:
    const uint8_t * mPayloadBufferData;
    const size_t mPayloadBufferLength;

public:
    /**
     * Constructs the Additional Data payload parser with payload buffer data
     * and the buffer size
     *
     * @param[in] payloadBufferData        The buffer data for the additional data payload,
     *                                     it needs to outlive the lifetime of this parse.
     * @param[in] payloadBufferLength      The buffer data length for the additional data payload.
     */
    AdditionalDataPayloadParser(const uint8_t * payloadBufferData, const size_t payloadBufferLength) :
        mPayloadBufferData(payloadBufferData), mPayloadBufferLength(payloadBufferLength)
    {}

    /**
     * Parses the Additional Data payload buffer and constructs all the fields
     * of the Additional Data structure.
     *
     * @param[out] outPayload              Additional data payload stucture.
     *
     *
     * @retval #CHIP_NO_ERROR              If the reader was successfully positioned on a new element.
     * @retval #CHIP_END_OF_TLV            If no further elements are available.
     * @retval #CHIP_ERROR_TLV_UNDERRUN    If the underlying TLV encoding ended prematurely.
     * @retval #CHIP_ERROR_INVALID_TLV_ELEMENT
     *                                      If the reader encountered an invalid or unsupported TLV element
     *                                      type.
     * @retval #CHIP_ERROR_INVALID_TLV_TAG If the reader encountered a TLV tag in an invalid context.
     * @retval #CHIP_ERROR_UNKNOWN_IMPLICIT_TLV_TAG
     *                                      If the reader encountered a implicitly-encoded TLV tag for which
     *                                      the corresponding profile id is unknown.
     * @retval other                        Other CHIP or platform error codes returned by the configured
     *                                      GetNextBuffer() function. Only possible when GetNextBuffer is
     *                                      non-NULL.
     */
    CHIP_ERROR populatePayload(SetupPayloadData::AdditionalDataPayload & outPayload);
};

} // namespace chip
