/*
 *
 *    <COPYRIGHT>
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
 *      This file implements converting an array of bytes into a Base45 String and
 *       the reverse.
 *
 *      The encoding chosen is: treat every 2 bytes of input data as a little-endian
 *        uint16_t, then div and mod that into 3 base45 characters, with the least-significant
 *        encoding bits in the first character of the resulting string.  If an odd
 *        number of bytes is encoded, 2 characters are used to encode the last byte.
 *
 */

#include "SetupCodeUtils.h"

using namespace std;

namespace chip {

static const int kBase45ChunkLen = 3;

string base45Encode(const uint8_t * buf, size_t buf_len)
{
    const char codes[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E',
                           'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
                           'U', 'V', 'W', 'X', 'Y', 'Z', ' ', '$', '%', '*', '+', '-', '.', '/', ':' };

    string result;
    int radix = sizeof(codes) / sizeof(codes[0]);

    // eat little-endian uint16_ts from the byte array
    // encode as 3 base45 characters
    while (buf_len >= 2)
    {
        int next = buf[0] + buf[1] * 256;

        for (int _ = 0; _ < kBase45ChunkLen; _++)
        {
            result += codes[next % radix];
            next /= radix;
        }
        buf += 2;
        buf_len -= 2;
    }
    // handle leftover byte, if any
    if (buf_len != 0)
    {
        int next = buf[0];
        for (int _ = 0; _ < kBase45ChunkLen - 1; _++)
        {
            result += codes[next % radix];
            next /= radix;
        }
    }
    return result;
}

static inline int decodeChar(char c, uint8_t & value)
{
    static const int kBogus = 255;
    // map of base45 charater to numeric value
    // subtract 32 from the charater, then index into this array, if possible
    const uint8_t decodes[] = {
        36,     // ' ', =32
        kBogus, // '!', =33
        kBogus, // '"', =34
        kBogus, // '#', =35
        37,     // '$', =36
        38,     // '%', =37
        kBogus, // '&', =38
        kBogus, // ''', =39
        kBogus, // '(', =40
        kBogus, // ')', =41
        39,     // '*', =42
        40,     // '+', =43
        kBogus, // ',', =44
        41,     // '-', =45
        42,     // '.', =46
        43,     // '/', =47
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
        44,     // ':', =58
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
        return 1;
    }
    uint8_t v = decodes[c - ' '];
    if (v == kBogus)
    {
        return 2;
    }
    value = v;
    return 0;
}

vector<uint8_t> base45Decode(string base45)
{
    // not long enough, there are always at least 2
    //  base45 characters
    if (base45.length() < kBase45ChunkLen - 1)
    {
        return vector<uint8_t>();
    }
    vector<uint8_t> result;

    for (int i = 0; base45.length() - i >= kBase45ChunkLen; i += kBase45ChunkLen)
    {
        uint8_t v0 = 0, v1 = 0, v2 = 0;

        if (decodeChar(base45[i], v0) || decodeChar(base45[i + 1], v1) || decodeChar(base45[i + 2], v2))
        {
            return vector<uint8_t>();
        }

        uint16_t value = v0 + v1 * 45 + v2 * 45 * 45;

        result.push_back(value % 256);
        result.push_back(value / 256);
    }
    if (base45.length() % kBase45ChunkLen == kBase45ChunkLen - 1)
    {
        int i      = base45.length() - (kBase45ChunkLen - 1);
        uint8_t v0 = 0, v1 = 0;

        if (decodeChar(base45[i], v0) || decodeChar(base45[i + 1], v1))
        {
            return vector<uint8_t>();
        }

        uint8_t value = v0 + v1 * 45;

        result.push_back(value);
    }
    return result;
}

} // namespace chip
