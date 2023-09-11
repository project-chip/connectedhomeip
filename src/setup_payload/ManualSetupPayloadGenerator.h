/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file describes a Manual Entry Code Generator based on the
 *      CHIP specification.
 *
 *      The encoding of the binary data to a decimal string is split
 *      into 5 chunks <1-digit/3-bits><5 digits/16-bits><4-digits/13-bits><5-digits/16-bits><5-digits/16-bits>:
 *      - <1 digit> Represents:
 *          - <bits 1..0> Discriminator <bits 11.10>
 *          - <bit 2> VID/PID present flag
 *      - <5 digits> Represents:
 *          - <bits 13..0> PIN Code <bits 13..0>
 *          - <bits 15..14> Discriminator <bits 9..8>
 *      - <4 digits> Represents:
 *          - <bits 12..0> PIN Code <bits 26..14>
 *      - <5 digits> Vendor ID
 *      - <5 digits> Product ID
 *
 */

#pragma once

#include "SetupPayload.h"

#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

#include <string>

namespace chip {

class ManualSetupPayloadGenerator
{
private:
    PayloadContents mPayloadContents;

public:
    ManualSetupPayloadGenerator(const PayloadContents & payload) : mPayloadContents(payload) {}

    /**
     * This function is called to encode the binary data of a payload to a
     * decimal null-terminated string.
     *
     * The resulting size of the outBuffer will be the size of data written and not including the null terminator.
     *
     * @param[out] outBuffer
     *                  Output buffer to write the decimal string.
     *
     * @retval #CHIP_NO_ERROR if the method succeeded.
     * @retval #CHIP_ERROR_INVALID_ARGUMENT if the payload is invalid.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL if outBuffer has insufficient size.
     */
    CHIP_ERROR payloadDecimalStringRepresentation(MutableCharSpan & outBuffer);

    // Populates decimal string representation of the payload into outDecimalString.
    // Wrapper for using std::string.
    CHIP_ERROR payloadDecimalStringRepresentation(std::string & outDecimalString);

    /**
     * This function disables internal checks about the validity of the generated payload.
     * It allows using the generator to generate invalid payloads.
     * Default is false.
     */
    void SetAllowInvalidPayload(bool allow) { mAllowInvalidPayload = allow; }

    /**
     * This function allow forcing the generation of a short code when the commissioning
     * flow is not standard by ignoring the vendor id and product id informations but with
     * the VID/PID present flag set.
     * Default is false.
     */
    void SetForceShortCode(bool useShort) { mForceShortCode = useShort; }

private:
    bool mAllowInvalidPayload = false;
    bool mForceShortCode      = false;
};

} // namespace chip
