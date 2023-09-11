/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <app/util/af.h>
#include <app/util/util.h>
#include <lib/core/CHIPEncoding.h>

using namespace chip;

uint8_t emberAfStringLength(const uint8_t * buffer)
{
    // The first byte specifies the length of the string.  A length of 0xFF means
    // the string is invalid and there is no character data.
    return (buffer[0] == 0xFF ? 0 : buffer[0]);
}

uint16_t emberAfLongStringLength(const uint8_t * buffer)
{
    // The first two bytes specify the length of the long string.  A length of
    // 0xFFFF means the string is invalid and there is no character data.
    uint16_t length = Encoding::LittleEndian::Get16(buffer);
    return (length == 0xFFFF ? 0 : length);
}
