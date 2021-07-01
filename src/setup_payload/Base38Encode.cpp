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

/**
 *    @file
 *      This file implements converting an array of bytes into a Base38 String.
 *
 *      The encoding chosen is: treat every 3 bytes of input data as a little-endian
 *        uint32_t, then div and mod that into 5 base38 characters, with the least-significant
 *        encoding bits in the first character of the resulting string. If a number of bytes
 *        is used that is not multiple of 3, then last 2 bytes are encoded to 4 base38 characters
 *        or last 1 byte is encoded to 2 base38 characters. Algoritm considers worst case size
 *        of bytes chunks and does not introduce code length optimization.
 *
 */

#include "Base38Encode.h"

#include <climits>
#include <cstring>

namespace {

static const uint8_t kMaxBytesSingleChunkLen = 3;

} // unnamed namespace

namespace chip {

CHIP_ERROR base38Encode(const uint8_t * in_buf, size_t in_buf_len, char * out_buf, size_t out_buf_len)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int out_idx    = 0;

    memset(out_buf, '\0', out_buf_len);

    while (in_buf_len > 0)
    {
        uint32_t value = 0;
        static_assert(sizeof(value) * CHAR_BIT >= kMaxBytesSingleChunkLen * 8, "value might overflow");

        size_t bytesInChunk = (in_buf_len >= kMaxBytesSingleChunkLen) ? kMaxBytesSingleChunkLen : in_buf_len;

        for (uint8_t byte = 0; byte < bytesInChunk; byte++)
        {
            value += static_cast<uint32_t>(in_buf[byte] << (8 * byte));
        }
        in_buf_len -= bytesInChunk;
        in_buf += bytesInChunk;

        // Without code length optimization there is constant characters number needed for specific chunk size.
        const int base38CharactersNeeded = kBase38CharactersNeededInNBytesChunk[bytesInChunk - 1];

        if (out_idx + base38CharactersNeeded >= static_cast<int>(out_buf_len))
        {
            err = CHIP_ERROR_BUFFER_TOO_SMALL;
            break;
        }

        for (uint8_t character = 0; character < base38CharactersNeeded; character++)
        {
            out_buf[out_idx++] = kCodes[value % kRadix];
            value /= kRadix;
        }
    }

    return err;
}

} // namespace chip
