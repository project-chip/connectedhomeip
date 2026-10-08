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
 *      Base-85 utility functions.
 *
 *      Compatible with Python's base64.b85encode / b85decode (RFC 1924 alphabet).
 *      Generally, blocks of 4 bytes are encoded into 5 characters, but arbitrary
 *      input lengths are supported via internal padding and truncation of the last block.
 *
 *      Note: Unlike Pythons b85decode, this decoder rejects non-canonical encodings.
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>

#include <stddef.h>
#include <stdint.h>

namespace chip {

/**
 * Returns the decoded length for a given Base85 encoded length.
 */
constexpr size_t Base85DecodedLength(size_t encodedLength)
{
    const size_t remainder = encodedLength % 5;
    return (encodedLength / 5) * 4 + (remainder ? remainder - 1 : 0);
}

/**
 * Returns the Base85 encoded length for a given input length, or SIZE_MAX
 * if inputLength would require an encoded size longer than SIZE_MAX.
 */
constexpr size_t Base85EncodedLength(size_t inputLength)
{
    constexpr size_t maxInputLength = Base85DecodedLength(SIZE_MAX);
    VerifyOrReturnValue(inputLength < maxInputLength, SIZE_MAX);
    const size_t remainder = inputLength % 4;
    return (inputLength / 4) * 5 + (remainder ? remainder + 1 : 0);
}

/**
 * Encode bytes to a Base85 string. The output will NOT be null-terminated.
 *
 * @param src       Input bytes to encode.
 * @param srcSize   Number of input bytes.
 * @param dest      Output buffer for the encoded string.
 * @param destSize  Size of the output buffer (must be >= Base85EncodedLength(srcSize)).
 *
 * Note: src and dest must not overlap
 *
 * @retval CHIP_ERROR_BUFFER_TOO_SMALL if the output buffer is too small.
 */
CHIP_ERROR BytesToBase85(const uint8_t * src, size_t srcSize, char * dest, size_t destSize);

/**
 * Decode a Base85 string to bytes.
 *
 * @param src       Input Base85 string.
 * @param srcSize   Length of the input string.
 * @param dest      Output buffer for decoded bytes.
 * @param destSize  Size of the output buffer (must be >= Base85DecodedLength(srcSize)).
 *
 * Note: src and dest must not overlap, except for the special case of
 * in-place decoding (dest == src), which is supported.
 *
 * @retval CHIP_ERROR_BUFFER_TOO_SMALL if the output buffer is too small.
 * @retval CHIP_ERROR_INVALID_ARGUMENT if the input contains invalid characters, has an
 *                                     invalid length, or invalid (non-canonical) padding.
 */
CHIP_ERROR Base85ToBytes(const char * src, size_t srcSize, uint8_t * dest, size_t destSize);

// Legacy API compatible with Base64Encode / Base64Decode.

inline uint16_t Base85Encode(const uint8_t * in, uint16_t inLen, char * out)
{
    size_t outLen = Base85EncodedLength(inLen);
    VerifyOrReturnValue(outLen < UINT16_MAX && BytesToBase85(in, inLen, out, outLen) == CHIP_NO_ERROR, UINT16_MAX);
    return static_cast<uint16_t>(outLen);
}

inline uint16_t Base85Decode(const char * in, uint16_t inLen, uint8_t * out)
{
    size_t outLen = Base85DecodedLength(inLen);
    VerifyOrReturnValue(outLen < UINT16_MAX && Base85ToBytes(in, inLen, out, outLen) == CHIP_NO_ERROR, UINT16_MAX);
    return static_cast<uint16_t>(outLen);
}

inline uint32_t Base85Encode32(const uint8_t * in, uint32_t inLen, char * out)
{
    size_t outLen = Base85EncodedLength(inLen);
    VerifyOrReturnValue(outLen < UINT32_MAX && BytesToBase85(in, inLen, out, outLen) == CHIP_NO_ERROR, UINT32_MAX);
    return static_cast<uint32_t>(outLen);
}

inline uint32_t Base85Decode32(const char * in, uint32_t inLen, uint8_t * out)
{
    size_t outLen = Base85DecodedLength(inLen);
    VerifyOrReturnValue(outLen < UINT32_MAX && Base85ToBytes(in, inLen, out, outLen) == CHIP_NO_ERROR, UINT32_MAX);
    return static_cast<uint32_t>(outLen);
}

} // namespace chip
