/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
#include <lib/core/CHIPEncoding.h>
#include <lib/support/CodeUtils.h>

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

    return static_cast<char>(x + '0');
}

CHIP_ERROR MakeU8FromAsciiHex(const char * src, const size_t srcLen, uint8_t * val, BitFlags<HexFlags> flags)
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
        else if (c >= 'A' && c <= 'F')
        {
            ret = static_cast<uint8_t>(ret + cval - static_cast<uint8_t>('A') + 0xA);
        }
        // If kUppercase flag is not set then lowercase are also allowed.
        else if (!flags.Has(HexFlags::kUppercase) && c >= 'a' && c <= 'f')
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

size_t HexToBytes(const char * src_hex, const size_t src_size, uint8_t * dest_bytes, size_t dest_size_max, BitFlags<HexFlags> flags)
{
    if ((src_hex == nullptr) || (dest_bytes == nullptr))
    {
        return 0;
    }
    // Octet string where each octet is 2 ascii digits representing the hex value
    // Each is represented by two ascii chars, so must be even number
    if ((src_size & 0x1) != 0 || src_size > dest_size_max * 2)
    {
        return 0;
    }

    size_t bytesFilled = 0;
    for (size_t i = 0; i < src_size; i += 2)
    {
        VerifyOrReturnError(MakeU8FromAsciiHex(src_hex + i, 2, &dest_bytes[i / 2], flags) == CHIP_NO_ERROR, 0);
        bytesFilled++;
    }
    return bytesFilled;
}

} // namespace

CHIP_ERROR BytesToHex(const uint8_t * src_bytes, size_t src_size, char * dest_hex, size_t dest_size_max, BitFlags<HexFlags> flags)
{
    if ((src_bytes == nullptr) && (src_size != 0))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if ((dest_hex == nullptr) && (dest_size_max != 0))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (src_size > ((SIZE_MAX - 1) / 2u))
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

CHIP_ERROR Uint64ToHex(uint64_t src, char * dest_hex, size_t dest_size_max, BitFlags<HexFlags> flags)
{
    uint8_t buf[sizeof(src)] = { 0 };
    Encoding::BigEndian::Put64(buf, src);
    return BytesToHex(buf, sizeof(buf), dest_hex, dest_size_max, flags);
}

CHIP_ERROR Uint32ToHex(uint32_t src, char * dest_hex, size_t dest_size_max, BitFlags<HexFlags> flags)
{
    uint8_t buf[sizeof(src)] = { 0 };
    Encoding::BigEndian::Put32(buf, src);
    return BytesToHex(buf, sizeof(buf), dest_hex, dest_size_max, flags);
}

CHIP_ERROR Uint16ToHex(uint16_t src, char * dest_hex, size_t dest_size_max, BitFlags<HexFlags> flags)
{
    uint8_t buf[sizeof(src)] = { 0 };
    Encoding::BigEndian::Put16(buf, src);
    return BytesToHex(buf, sizeof(buf), dest_hex, dest_size_max, flags);
}

size_t HexToBytes(const char * src_hex, const size_t src_size, uint8_t * dest_bytes, size_t dest_size_max)
{
    return HexToBytes(src_hex, src_size, dest_bytes, dest_size_max, HexFlags::kNone);
}

size_t UppercaseHexToUint64(const char * src_hex, const size_t src_size, uint64_t & dest)
{
    uint8_t buf[sizeof(uint64_t)] = { 0 };
    size_t decoded_size           = HexToBytes(src_hex, src_size, buf, sizeof(buf), HexFlags::kUppercase);
    if (decoded_size != sizeof(buf))
    {
        return 0;
    }
    dest = Encoding::BigEndian::Get64(buf);
    return decoded_size;
}

size_t UppercaseHexToUint32(const char * src_hex, const size_t src_size, uint32_t & dest)
{
    uint8_t buf[sizeof(uint32_t)] = { 0 };
    size_t decoded_size           = HexToBytes(src_hex, src_size, buf, sizeof(buf), HexFlags::kUppercase);
    if (decoded_size != sizeof(buf))
    {
        return 0;
    }
    dest = Encoding::BigEndian::Get32(buf);
    return decoded_size;
}

size_t UppercaseHexToUint16(const char * src_hex, const size_t src_size, uint16_t & dest)
{
    uint8_t buf[sizeof(uint16_t)] = { 0 };
    size_t decoded_size           = HexToBytes(src_hex, src_size, buf, sizeof(buf), HexFlags::kUppercase);
    if (decoded_size != sizeof(buf))
    {
        return 0;
    }
    dest = Encoding::BigEndian::Get16(buf);
    return decoded_size;
}

void LogBufferAsHex(const char * label, const ByteSpan & span)
{
    constexpr size_t kBytesPerLine = 32u;

    size_t remaining = span.size();
    if (remaining == 0)
    {
        ChipLogProgress(Support, "%s>>>", ((label != nullptr) ? label : ""));
        return;
    }

    const uint8_t * cursor = span.data();
    while (remaining > 0u)
    {
        size_t chunk_size = (remaining < kBytesPerLine) ? remaining : kBytesPerLine;
        char hex_buf[(kBytesPerLine * 2) + 1];

        CHIP_ERROR err = BytesToUppercaseHexString(cursor, chunk_size, &hex_buf[0], sizeof(hex_buf));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogProgress(Support, "Failed to dump hex %" CHIP_ERROR_FORMAT, err.Format());
            return;
        }

        ChipLogProgress(Support, "%s>>>%s", ((label != nullptr) ? label : ""), hex_buf);

        cursor += chunk_size;
        remaining -= chunk_size;
    }
}

} // namespace Encoding
} // namespace chip
