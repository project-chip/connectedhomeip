/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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
 *      Base-85 utility functions (RFC 1924 alphabet, git/Python compatible).
 */

#include "Base85.h"

#include <string.h>

namespace chip {

// RFC 1924 alphabet: 0-9 A-Z a-z !#$%&()*+-;<=>?@^_`{|}~
// Values 0-9 map to '0'-'9', 10-35 to 'A'-'Z', 36-61 to 'a'-'z',
// and values 62-84 map to the 23 punctuation characters below.

// Convert a value in the range 0..84 to its RFC 1924 base85 character.
static char Base85ValToChar(uint8_t val)
{
    switch (val)
    {
    // clang-format off
    case 62: return '!';
    case 63: return '#';
    case 64: return '$';
    case 65: return '%';
    case 66: return '&';
    case 67: return '(';
    case 68: return ')';
    case 69: return '*';
    case 70: return '+';
    case 71: return '-';
    case 72: return ';';
    case 73: return '<';
    case 74: return '=';
    case 75: return '>';
    case 76: return '?';
    case 77: return '@';
    case 78: return '^';
    case 79: return '_';
    case 80: return '`';
    case 81: return '{';
    case 82: return '|';
    case 83: return '}';
    case 84: return '~';
    // clang-format on
    default:
        if (val < 10)
        {
            return static_cast<char>('0' + val);
        }
        if (10 <= val && val < 36)
        {
            return static_cast<char>('A' + val - 10);
        }
        if (36 <= val && val < 62)
        {
            return static_cast<char>('a' + val - 36);
        }
    }
    return 0; // not possible
}

// Convert an RFC 1924 base85 character to a value in the range 0..84,
// or UINT8_MAX if the character is not in the alphabet.
static uint8_t Base85CharToVal(char c)
{
    switch (c)
    {
    // clang-format off
    case '!': return 62;
    case '#': return 63;
    case '$': return 64;
    case '%': return 65;
    case '&': return 66;
    case '(': return 67;
    case ')': return 68;
    case '*': return 69;
    case '+': return 70;
    case '-': return 71;
    case ';': return 72;
    case '<': return 73;
    case '=': return 74;
    case '>': return 75;
    case '?': return 76;
    case '@': return 77;
    case '^': return 78;
    case '_': return 79;
    case '`': return 80;
    case '{': return 81;
    case '|': return 82;
    case '}': return 83;
    case '~': return 84;
    // clang-format on
    default:
        if (c >= '0' && c <= '9')
        {
            return static_cast<uint8_t>(c - '0');
        }
        if (c >= 'A' && c <= 'Z')
        {
            return static_cast<uint8_t>(c - 'A' + 10);
        }
        if (c >= 'a' && c <= 'z')
        {
            return static_cast<uint8_t>(c - 'a' + 36);
        }
    }
    return UINT8_MAX;
}

static uint32_t ReadGroup(const uint8_t * src, size_t count)
{
    uint8_t padded[4] = { 0 };
    memcpy(padded, src, count);
    return (static_cast<uint32_t>(padded[0]) << 24) | //
        (static_cast<uint32_t>(padded[1]) << 16) |    //
        (static_cast<uint32_t>(padded[2]) << 8) |     //
        (static_cast<uint32_t>(padded[3]));
}

static void WriteGroup(uint32_t value, uint8_t * dest, size_t count)
{
    uint8_t decoded[4] = {
        static_cast<uint8_t>(value >> 24),
        static_cast<uint8_t>(value >> 16),
        static_cast<uint8_t>(value >> 8),
        static_cast<uint8_t>(value),
    };
    memcpy(dest, decoded, count);
}

static bool DecodeGroup(const char * src, size_t count, uint32_t & outValue)
{
    // "|NsC0" is the largest allowed group value (UINT32_MAX); we could
    // check against this value here directly, but it is simpler to do the
    // calculation in a uint64_t and validate the decoded result.
    uint64_t value = 0;
    for (size_t j = 0; j < count; j++)
    {
        uint8_t digit = Base85CharToVal(src[j]);
        VerifyOrReturnValue(digit != UINT8_MAX, false);
        value = value * 85 + digit;
    }
    for (size_t j = count; j < 5; j++)
    {
        value = value * 85 + 84; // implicitly pad to 5 "digits" with '~' (84)
    }
    VerifyOrReturnValue(value <= UINT32_MAX, false);
    outValue = static_cast<uint32_t>(value);
    return true;
}

static void EncodeGroup(uint32_t value, char * out, size_t count)
{
    for (size_t j = 5; j-- > count;)
    {
        value /= 85; // truncate least significant "digits"
    }
    for (size_t j = count; j-- > 0;)
    {
        out[j] = Base85ValToChar(static_cast<uint8_t>(value % 85));
        value /= 85;
    }
}

CHIP_ERROR BytesToBase85(const uint8_t * src, size_t srcSize, char * dest, size_t destSize)
{
    VerifyOrReturnError(src != nullptr || srcSize == 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(dest != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    // Base85EncodedLength(srcSize) could overflow / saturate to SIZE_MAX,
    // checking via Base85DecodedLength(destSize) instead avoids this issue.
    VerifyOrReturnError(Base85DecodedLength(destSize) >= srcSize, CHIP_ERROR_BUFFER_TOO_SMALL);

    size_t remainder = srcSize % 4;
    for (auto * end = src + (srcSize - remainder); src < end; src += 4, dest += 5)
    {
        EncodeGroup(ReadGroup(src, 4), dest, 5);
    }
    if (remainder > 0)
    {
        EncodeGroup(ReadGroup(src, remainder), dest, remainder + 1);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Base85ToBytes(const char * src, size_t srcSize, uint8_t * dest, size_t destSize)
{
    VerifyOrReturnError(src != nullptr || srcSize == 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(dest != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(destSize >= Base85DecodedLength(srcSize), CHIP_ERROR_BUFFER_TOO_SMALL);

    uint32_t value;
    size_t remainder = srcSize % 5;
    VerifyOrReturnError(remainder != 1, CHIP_ERROR_INVALID_ARGUMENT); // 1 "digit" does not encode a full byte
    for (auto * end = src + (srcSize - remainder); src < end; src += 5, dest += 4)
    {
        VerifyOrReturnError(DecodeGroup(src, 5, value), CHIP_ERROR_INVALID_ARGUMENT);
        WriteGroup(value, dest, 4);
    }
    if (remainder > 0) // 2..4
    {
        VerifyOrReturnError(DecodeGroup(src, remainder, value), CHIP_ERROR_INVALID_ARGUMENT);

        char check[4];
        uint32_t padding = UINT32_MAX >> (8 * (remainder - 1));
        EncodeGroup(value & ~padding, check, remainder);
        VerifyOrReturnError(memcmp(src, check, remainder) == 0, CHIP_ERROR_INVALID_ARGUMENT);

        WriteGroup(value, dest, remainder - 1); // might clobber src
    }
    return CHIP_NO_ERROR;
}

} // namespace chip
