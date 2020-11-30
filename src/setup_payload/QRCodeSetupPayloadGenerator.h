/*
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
 *      This file describes a QRCode Setup Payload generator based on the
 *      CHIP specification.
 *
 *      The encoding of the binary data to a base41 string is as follows:
 *      - Every 2 bytes (16 bits) of binary source data are encoded to 3
 *        characters of the Base-41 alphabet.
 *      - If an odd number of bytes are to be encoded, the remaining
 *        single byte is encoded to 2 characters of the Base-41 alphabet.
 */

#include "SetupPayload.h"

#include <string>

#pragma once

namespace chip {

class QRCodeSetupPayloadGenerator
{
private:
    SetupPayload mPayload;

public:
    QRCodeSetupPayloadGenerator(const SetupPayload & setupPayload) : mPayload(setupPayload) {}

    /**
     * This function is called to encode the binary data of a payload to a
     * base41 string using CHIP TLV encoding scheme.
     *
     * @param[out] base41Representation
     *                  The string to copy the base41 to.
     *
     * @retval #CHIP_NO_ERROR if the method succeeded.
     * @retval #CHIP_ERROR_INVALID_ARGUMENT if the payload is invalid.
     * @retval other Other CHIP or platform-specific error codes indicating
     *               that an error occurred preventing the function from
     *               producing the requested string.
     */
    CHIP_ERROR payloadBase41Representation(std::string & base41Representation);

    /**
     * This function is called to encode the binary data of a payload to a
     * base41 string. Callers must pass a buffer of at least
     * chip::kTotalPayloadDataInBytes or more if there is any serialNumber or
     * any other optional data. The buffer should be big enough to hold the
     * TLV encoded value of the payload. If not an error will be throw.
     *
     * @param[out] base41Representation
     *                  The string to copy the base41 to.
     * @param[in]  tlvDataStart
     *                  A pointer to an uint8_t buffer into which the TLV
     *                  should be written.
     * @param[in]  tlvDataStartSize
     *                  The maximum number of bytes that should be written to
     *                  the output buffer.
     *
     * @retval #CHIP_NO_ERROR if the method succeeded.
     * @retval #CHIP_ERROR_INVALID_ARGUMENT if the payload is invalid.
     * @retval other Other CHIP or platform-specific error codes indicating
     *               that an error occurred preventing the function from
     *               producing the requested string.
     */
    CHIP_ERROR payloadBase41Representation(std::string & base41Representation, uint8_t * tlvDataStart, uint32_t tlvDataStartSize);

private:
    CHIP_ERROR generateTLVFromOptionalData(SetupPayload & outPayload, uint8_t * tlvDataStart, uint32_t maxLen,
                                           size_t & tlvDataLengthInBytes);
};

} // namespace chip
