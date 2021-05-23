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
 *      This file implements converting an array of bytes into a Base41 String and
 *       the reverse.
 *
 *      The encoding chosen is: treat every 2 bytes of input data as a little-endian
 *        uint16_t, then div and mod that into 3 base41 characters, with the least-significant
 *        encoding bits in the first character of the resulting string.  If an odd
 *        number of bytes is encoded, 2 characters are used to encode the last byte.
 *
 */

#include "Base41.h"

#include <climits>

namespace chip {

constexpr int kBase41ChunkLen = 3;
constexpr int kBytesChunkLen  = 2;

std::string base41Encode(const uint8_t * buf, size_t buf_len)
{
    std::string result;

    // eat little-endian uint16_ts from the byte array
    // encode as 3 base41 characters

    while (buf_len >= kBytesChunkLen)
    {
        int value = 0;
        for (int i = kBytesChunkLen - 1; i >= 0; i--)
        {
            value *= 256;
            value += buf[i];
        }
        buf_len -= kBytesChunkLen;
        buf += kBytesChunkLen;

        // This code needs to correctly convey to the decoder
        //  the length of data encoded, so normally emits 3 chars for
        //  2 bytes.
        // But there's a special case possible where the last
        //  two bytes would fit in exactly 2 chars.
        // The following conditions must be met:
        //   1. this must be the last value
        //   2. the value doesn't fit in a single byte, if we encode a
        //        small value at the end of the encoded string with a
        //        shortened chunk, the decoder will think only one byte
        //        was encoded
        //   3. the value can be encoded in 2 base41 chars
        //
        int encodeLen = kBase41ChunkLen;
        if (buf_len == 0 &&            // the very last value, i.e. not an odd byte
            (value > UINT8_MAX) &&     // the value wouldn't fit in one byte
            (value < kRadix * kRadix)) // the value can be encoded with 2 base41 chars
        {
            encodeLen--;
        }
        for (int _ = 0; _ < encodeLen; _++)
        {
            result += codes[value % kRadix];
            value /= kRadix;
        }
    }

    // handle leftover bytes, if any
    if (buf_len != 0)
    {
        uint64_t value = 0;
        static_assert(sizeof(value) * CHAR_BIT >= kBytesChunkLen * 8, "value might overflow");

        for (size_t i = buf_len; i > 0; i--)
        {
            value *= 256;
            value += buf[i - 1];
        }

        // need to indicate there are leftover bytes, so append at least one encoding char
        result += codes[value % kRadix];
        value /= kRadix;

        // if there's still value, encode with more chars
        while (value != 0)
        {
            result += codes[value % kRadix];
            value /= kRadix;
        }
    }
    return result;
}

static inline CHIP_ERROR decodeChar(char c, uint8_t & value)
{
    static const int kBogus = 255;
    // map of base41 charater to numeric value
    // subtract 32 from the charater, then index into this array, if possible
    const uint8_t decodes[] = {
        36,     // ' ', =32
        kBogus, // '!', =33
        kBogus, // '"', =34
        kBogus, // '#', =35
        kBogus, // '$', =36
        kBogus, // '%', =37
        kBogus, // '&', =38
        kBogus, // ''', =39
        kBogus, // '(', =40
        kBogus, // ')', =41
        kBogus, // '*', =42
        37,     // '+', =43
        kBogus, // ',', =44
        38,     // '-', =45
        39,     // '.', =46
        40,     // '/', =47
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
    if (c < ' ' || c > 'Z')
    {
        return CHIP_ERROR_INVALID_INTEGER_VALUE;
    }
    uint8_t v = decodes[c - ' '];
    if (v == kBogus)
    {
        return CHIP_ERROR_INVALID_INTEGER_VALUE;
    }
    value = v;
    return 0;
}

CHIP_ERROR base41Decode(std::string base41, std::vector<uint8_t> & result)
{
    result.clear();

    for (size_t i = 0; base41.length() - i >= kBase41ChunkLen; i += kBase41ChunkLen)
    {
        uint16_t value = 0;

        for (size_t iv = i + kBase41ChunkLen; iv > i; iv--)
        {
            uint8_t v;
            CHIP_ERROR err = decodeChar(base41[iv - 1], v);

            if (err != CHIP_NO_ERROR)
            {
                return err;
            }

            value = static_cast<uint16_t>(value * kRadix + v);
        }

        result.push_back(static_cast<uint8_t>(value % 256));
        // Cast is safe, because we divided a uint16_t by 256 to get here,
        // so what's left has to fit inside uint8_t.
        result.push_back(static_cast<uint8_t>(value / 256));
    }

    if (base41.length() % kBase41ChunkLen != 0) // only 1 or 2 chars left
    {
        size_t tail    = (base41.length() % kBase41ChunkLen);
        size_t i       = base41.length() - tail;
        uint16_t value = 0;

        for (size_t iv = base41.length(); iv > i; iv--)
        {
            uint8_t v;
            CHIP_ERROR err = decodeChar(base41[iv - 1], v);

            if (err != CHIP_NO_ERROR)
            {
                return err;
            }

            value = static_cast<uint16_t>(value * kRadix + v);
        }
        result.push_back(static_cast<uint8_t>(value % 256));
        value /= 256;
        if (value != 0)
        {
            // Cast is safe, because we divided a uint16_t by 256 to get here,
            // so what's left has to fit inside uint8_t.
            result.push_back(static_cast<uint8_t>(value));
        }
    }
    return CHIP_NO_ERROR;
}

} // namespace chip
