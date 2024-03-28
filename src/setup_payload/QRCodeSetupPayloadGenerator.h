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
 *      The encoding of the binary data to a base38 string is as follows:
 *      - Every 2 bytes (16 bits) of binary source data are encoded to 3
 *        characters of the Base-38 alphabet.
 *      - If an odd number of bytes are to be encoded, the remaining
 *        single byte is encoded to 2 characters of the Base-38 alphabet.
 */

#include "SetupPayload.h"

#include <lib/support/Span.h>

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
     * base38 null-terminated string.
     *
     * If the payload has any optional data that needs to be TLV encoded, this
     * function will fail.
     *
     * @param[out] base38Representation
     *                  The string to copy the base38 to.
     *
     * @retval #CHIP_NO_ERROR if the method succeeded.
     * @retval #CHIP_ERROR_INVALID_ARGUMENT if the payload is invalid.
     * @retval other Other CHIP or platform-specific error codes indicating
     *               that an error occurred preventing the function from
     *               producing the requested string.
     */
    CHIP_ERROR payloadBase38Representation(std::string & base38Representation);

    /**
     * This function is called to encode the binary data of a payload to a
     * base38 null-terminated string.
     *
     * If the payload has any optional data that needs to be TLV encoded, this
     * function will allocate a scratch heap buffer to hold the TLV data while
     * encoding.
     *
     * @param[out] base38Representation
     *                  The string to copy the base38 to.
     *
     * @retval #CHIP_NO_ERROR if the method succeeded.
     * @retval #CHIP_ERROR_INVALID_ARGUMENT if the payload is invalid.
     * @retval other Other CHIP or platform-specific error codes indicating
     *               that an error occurred preventing the function from
     *               producing the requested string.
     */
    CHIP_ERROR payloadBase38RepresentationWithAutoTLVBuffer(std::string & base38Representation);

    /**
     * This function is called to encode the binary data of a payload to a
     * base38 null-terminated string, using the caller-provided buffer as
     * temporary scratch space for optional data that needs to be TLV-encoded.
     * If that buffer is not big enough to hold the TLV-encoded part of the
     * payload, this function will fail.
     *
     * @param[out] base38Representation
     *                  The string to copy the base38 to.
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
    CHIP_ERROR payloadBase38Representation(std::string & base38Representation, uint8_t * tlvDataStart, uint32_t tlvDataStartSize);

    /**
     * This function disables internal checks about the validity of the generated payload.
     * It allows using the generator to generate invalid payloads.
     * Default is false.
     */
    void SetAllowInvalidPayload(bool allow) { mAllowInvalidPayload = allow; }

private:
    CHIP_ERROR generateTLVFromOptionalData(SetupPayload & outPayload, uint8_t * tlvDataStart, uint32_t maxLen,
                                           size_t & tlvDataLengthInBytes);

    bool mAllowInvalidPayload = false;
};

/**
 * A minimal QR code setup payload generator that omits any optional data,
 * for compatibility with devices that don't support std::string or STL.
 */
class QRCodeBasicSetupPayloadGenerator
{
private:
    PayloadContents mPayload;

public:
    QRCodeBasicSetupPayloadGenerator(const PayloadContents & payload) : mPayload(payload) {}

    /**
     * This function is called to encode the binary data of a payload to a
     * base38 null-terminated string.
     *
     * The resulting size of the out_buf span will be the size of data written
     * and not including the null terminator.
     *
     * This function will fail if the payload has any optional data requiring
     * TLV encoding.
     *
     * @param[out] outBuffer
     *                  The buffer to copy the base38 to.
     *
     * @retval #CHIP_NO_ERROR if the method succeeded.
     * @retval #CHIP_ERROR_INVALID_ARGUMENT if the payload is invalid.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL if outBuffer has insufficient size.
     * @retval other Other CHIP or platform-specific error codes indicating
     *               that an error occurred preventing the function from
     *               producing the requested string.
     */
    CHIP_ERROR payloadBase38Representation(MutableCharSpan & outBuffer);

    // TODO - Find the optimal value for maximum length of QR Code Base38 string
    static constexpr uint16_t kMaxQRCodeBase38RepresentationLength = 128;
};

} // namespace chip
