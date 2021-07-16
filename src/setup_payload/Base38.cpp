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
 *      This file implements converting an array of bytes into a Base38 String and
 *       the reverse.
 *
 *      The encoding chosen is: treat every 3 bytes of input data as a little-endian
 *        uint32_t, then div and mod that into 5 base38 characters, with the least-significant
 *        encoding bits in the first character of the resulting string. If a number of bytes
 *        is used that is not multiple of 3, then last 2 bytes are encoded to 4 base38 characters
 *        or last 1 byte is encoded to 2 base38 characters. Algoritm considers worst case size
 *        of bytes chunks and does not introduce code length optimization.
 *
 */

#include "Base38.h"

#include <climits>

namespace {

static const char kCodes[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
                               'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '-', '.' };
static const uint8_t kBase38CharactersNeededInNBytesChunk[] = { 2, 4, 5 };
static const uint8_t kMaxBytesSingleChunkLen                = 3;
static const uint8_t kRadix                                 = sizeof(kCodes) / sizeof(kCodes[0]);

static inline CHIP_ERROR decodeChar(char c, uint8_t & value)
{
    static const int kBogus = 255;
    // map of base38 charater to numeric value
    // subtract 45 from the charater, then index into this array, if possible
    const uint8_t decodes[] = {
        36,     // '-', =45
        37,     // '.', =46
        kBogus, // '/', =47
        0,      // '0', =48
        1,      // '1', =49
        2,      // '2', =50
        3,      // '3', =51
        4,      // '4', =52
        5,      // '5', =53
        6,      // '6', =54
        7,      // '7', =55
        8,      // '8', =56
        9,      // '9', =57
        kBogus, // ':', =58
        kBogus, // ';', =59
        kBogus, // '<', =50
        kBogus, // '=', =61
        kBogus, // '>', =62
        kBogus, // '?', =63
        kBogus, // '@', =64
        10,     // 'A', =65
        11,     // 'B', =66
        12,     // 'C', =67
        13,     // 'D', =68
        14,     // 'E', =69
        15,     // 'F', =70
        16,     // 'G', =71
        17,     // 'H', =72
        18,     // 'I', =73
        19,     // 'J', =74
        20,     // 'K', =75
        21,     // 'L', =76
        22,     // 'M', =77
        23,     // 'N', =78
        24,     // 'O', =79
        25,     // 'P', =80
        26,     // 'Q', =81
        27,     // 'R', =82
        28,     // 'S', =83
        29,     // 'T', =84
        30,     // 'U', =85
        31,     // 'V', =86
        32,     // 'W', =87
        33,     // 'X', =88
        34,     // 'Y', =89
        35,     // 'Z', =90
    };
    if (c < '-' || c > 'Z')
    {
        return CHIP_ERROR_INVALID_INTEGER_VALUE;
    }
    uint8_t v = decodes[c - '-'];
    if (v == kBogus)
    {
        return CHIP_ERROR_INVALID_INTEGER_VALUE;
    }
    value = v;
    return CHIP_NO_ERROR;
}

} // unnamed namespace

namespace chip {

std::string base38Encode(const uint8_t * buf, size_t buf_len)
{
    std::string result;

    while (buf_len > 0)
    {
        uint32_t value = 0;
        static_assert(sizeof(value) * CHAR_BIT >= kMaxBytesSingleChunkLen * 8, "value might overflow");

        size_t bytesInChunk = (buf_len >= kMaxBytesSingleChunkLen) ? kMaxBytesSingleChunkLen : buf_len;

        for (uint8_t byte = 0; byte < bytesInChunk; byte++)
        {
            value += static_cast<uint32_t>(buf[byte] << (8 * byte));
        }
        buf_len -= bytesInChunk;
        buf += bytesInChunk;

        // Without code length optimization there is constant characters number needed for specific chunk size.
        const int base38CharactersNeeded = kBase38CharactersNeededInNBytesChunk[bytesInChunk - 1];

        for (uint8_t character = 0; character < base38CharactersNeeded; character++)
        {
            result += kCodes[value % kRadix];
            value /= kRadix;
        }
    }
    return result;
}

CHIP_ERROR base38Decode(std::string base38, std::vector<uint8_t> & result)
{
    result.clear();

    int base38CharactersNumber  = static_cast<int>(base38.length());
    int decodedBase38Characters = 0;
    while (base38CharactersNumber > 0)
    {
        uint8_t base38CharactersInChunk;
        uint8_t bytesInDecodedChunk;

        if (base38CharactersNumber >= kBase38CharactersNeededInNBytesChunk[2])
        {
            base38CharactersInChunk = kBase38CharactersNeededInNBytesChunk[2];
            bytesInDecodedChunk     = 3;
        }
        else if (base38CharactersNumber == kBase38CharactersNeededInNBytesChunk[1])
        {
            base38CharactersInChunk = kBase38CharactersNeededInNBytesChunk[1];
            bytesInDecodedChunk     = 2;
        }
        else if (base38CharactersNumber == kBase38CharactersNeededInNBytesChunk[0])
        {
            base38CharactersInChunk = kBase38CharactersNeededInNBytesChunk[0];
            bytesInDecodedChunk     = 1;
        }
        else
        {
            return CHIP_ERROR_INVALID_STRING_LENGTH;
        }

        uint32_t value = 0;

        for (int i = (base38CharactersInChunk - 1); i >= 0; i--)
        {
            uint8_t v;
            CHIP_ERROR err = decodeChar(base38[static_cast<uint8_t>(decodedBase38Characters + i)], v);

            if (err != CHIP_NO_ERROR)
            {
                return err;
            }

            value = value * kRadix + v;
        }
        decodedBase38Characters += base38CharactersInChunk;
        base38CharactersNumber -= base38CharactersInChunk;

        for (int i = 0; i < bytesInDecodedChunk; i++)
        {
            result.push_back(static_cast<uint8_t>(value));
            value >>= 8;
        }

        if (value > 0)
        {
            // encoded value is too big to represent a correct chunk of size 1, 2 or 3 bytes
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
    return CHIP_NO_ERROR;
}

} // namespace chip
