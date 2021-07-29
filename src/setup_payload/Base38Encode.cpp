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

CHIP_ERROR base38Encode(ByteSpan in_buf, MutableCharSpan & out_buf)
{
    CHIP_ERROR err             = CHIP_NO_ERROR;
    const uint8_t * in_buf_ptr = in_buf.data();
    size_t in_buf_len          = in_buf.size();
    size_t out_idx             = 0;

    while (in_buf_len > 0)
    {
        uint32_t value = 0;
        static_assert((sizeof(value) * CHAR_BIT) >= (kMaxBytesSingleChunkLen * 8), "Type for value is too small for conversions");

        size_t bytesInChunk = (in_buf_len >= kMaxBytesSingleChunkLen) ? kMaxBytesSingleChunkLen : in_buf_len;

        for (uint8_t byte_idx = 0; byte_idx < bytesInChunk; byte_idx++)
        {
            value += static_cast<uint32_t>(in_buf_ptr[byte_idx] << (8 * byte_idx));
        }
        in_buf_len -= bytesInChunk;
        in_buf_ptr += bytesInChunk;

        // Without code length optimization there is constant characters number needed for specific chunk size.
        const uint8_t base38CharactersNeeded = kBase38CharactersNeededInNBytesChunk[bytesInChunk - 1];

        if ((out_idx + base38CharactersNeeded) >= out_buf.size())
        {
            err = CHIP_ERROR_BUFFER_TOO_SMALL;
            break;
        }

        for (uint8_t character = 0; character < base38CharactersNeeded; character++)
        {
            out_buf.data()[out_idx++] = kCodes[value % kRadix];
            value /= kRadix;
        }
    }

    if (out_idx < out_buf.size())
    {
        out_buf.data()[out_idx] = '\0';
    }
    else
    {
        // out_buf size is zero.
        err = CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    return err;
}

size_t base38EncodedLength(size_t num_bytes)
{
    // Each group of 3 bytes converts to 5 chars, and each remaining byte converts to 2 chars.
    // Add one for the null terminator.
    return (num_bytes / 3) * 5 + (num_bytes % 3) * 2 + 1;
}

} // namespace chip
