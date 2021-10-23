/*
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

#include "BytesToHex.h"

#include <cstring>
#include <stdio.h>
namespace chip {
namespace Encoding {

namespace {

char NibbleToHex(uint8_t nibble, bool uppercase)
{
    char x = static_cast<char>(nibble & 0xFu);

    if (x >= 10)
    {
        return static_cast<char>((x - 10) + (uppercase ? 'A' : 'a'));
    }
    else
    {
        return static_cast<char>(x + '0');
    }
}

CHIP_ERROR MakeU8FromAsciiHex(const char * src, const size_t srcLen, uint8_t * val)
{
    if (srcLen != 2)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    uint8_t ret = 0;
    for (size_t i = 0; i < srcLen; ++i)
    {
        ret          = static_cast<uint8_t>(ret << 4);
        char c       = src[i];
        uint8_t cval = static_cast<uint8_t>(c);
        if (c >= '0' && c <= '9')
        {
            ret = static_cast<uint8_t>(ret + cval - static_cast<uint8_t>('0'));
        }
        // Only uppercase is supported according to spec.
        else if (c >= 'A' && c <= 'F')
        {
            ret = static_cast<uint8_t>(ret + cval - static_cast<uint8_t>('A') + 0xA);
        }
        else if (c >= 'a' && c <= 'f')
        {
            ret = static_cast<uint8_t>(ret + cval - static_cast<uint8_t>('a') + 0xA);
        }
        else
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
    *val = ret;
    return CHIP_NO_ERROR;
}

} // namespace

CHIP_ERROR BytesToHex(const uint8_t * src_bytes, size_t src_size, char * dest_hex, size_t dest_size_max, BitFlags<HexFlags> flags)
{
    if ((src_bytes == nullptr) || (dest_hex == nullptr))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    else if (src_size > ((SIZE_MAX - 1) / 2u))
    {
        // Output would overflow a size_t, let's bail out to avoid computation wraparounds below.
        // This condition will hit with slightly less than the very max, but is unlikely to
        // ever happen unless an error occurs and won't happen on embedded targets.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    bool nul_terminate          = flags.Has(HexFlags::kNullTerminate);
    size_t expected_output_size = (src_size * 2u) + (nul_terminate ? 1u : 0u);
    if (dest_size_max < expected_output_size)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    bool uppercase = flags.Has(HexFlags::kUppercase);
    char * cursor  = dest_hex;
    for (size_t byte_idx = 0; byte_idx < src_size; ++byte_idx)
    {
        *cursor++ = NibbleToHex((src_bytes[byte_idx] >> 4) & 0xFu, uppercase);
        *cursor++ = NibbleToHex((src_bytes[byte_idx] >> 0) & 0xFu, uppercase);
    }

    if (nul_terminate)
    {
        *cursor = '\0';
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR BytesToHex(uint64_t src, char * dest_hex, size_t dest_size_max, BitFlags<HexFlags> flags)
{
    uint8_t buf[8];
    for (int i = 7; i >= 0; --i)
    {
        buf[i] = src & 0xFF;
        src    = src >> 8;
    }

    return BytesToHex(buf, 8, dest_hex, dest_size_max, flags);
}

size_t HexToBytes(const char * srcHex, const size_t srcLen, uint8_t * destBytes, size_t destMaxLen)
{
    if ((srcHex == nullptr) || (destBytes == nullptr))
    {
        return 0;
    }
    // Octet string where each octet is 2 ascii digits representing the hex value
    // Each is represented by two ascii chars, so must be even number
    if ((srcLen & 0x1) != 0 || srcLen > destMaxLen * 2)
    {
        return 0;
    }

    memset(destBytes, 0, destMaxLen);
    size_t bytesFilled = 0;

    for (size_t i = 0; i < srcLen; i += 2)
    {
        if (MakeU8FromAsciiHex(srcHex + i, 2, &destBytes[i / 2]) != CHIP_NO_ERROR)
        {
            memset(destBytes, 0, destMaxLen);
            return 0;
        }
        bytesFilled++;
    }
    return bytesFilled;
}

} // namespace Encoding
} // namespace chip
