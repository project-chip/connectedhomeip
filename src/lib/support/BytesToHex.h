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

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/BitFlags.h>
#include <lib/support/Span.h>

#include <stdint.h>
#include <stdlib.h>

namespace chip {
namespace Encoding {

enum class HexFlags : int
{
    kNone = 0u,
    // Use uppercase A-F if set otherwise, lowercase a-f
    kUppercase = (1u << 0),
    // Null-terminate buffer
    kNullTerminate = (1u << 1),
    // Both use uppercase and null-termination.
    // Separately stated to avoid casts for common case.
    kUppercaseAndNullTerminate = ((1u << 0) | (1u << 1))
};

/**
 * Encode a buffer of bytes into hexadecimal, with or without null-termination
 * and using either lowercase or uppercase hex. The input bytes are assumed to be
 * in a big-engian order. The output is also in a big-endian order.
 *
 * Default is lowercase output, not null-terminated.
 *
 * If `flags` has `HexFlags::kNullTerminate` set, treat `dest_hex` as a
 * null-terminated string buffer. The function returns CHIP_ERROR_BUFFER_TOO_SMALL
 * if `dest_size_max` can't fit the entire encoded buffer, and the
 * null-terminator if enabled. This function will never output truncated data.
 * The result either fits and is written, or does not fit and nothing is written
 * to `dest_hex`.
 *
 * If `src_bytes` and `dest_hex` overlap, the results may be incorrect, depending
 * on overlap, but only the core validity checks are done and it's possible to
 * get CHIP_NO_ERROR with erroneous output.
 *
 * On success, number of bytes written to destination is always:
 *   output_size = (src_size * 2) + ((flags & HexFlags::kNullTerminate) ? 1 : 0);
 *
 * @param src_bytes Pointer to buffer to convert.  Only allowed to be null if
 *                  src_size is 0.
 * @param src_size Number of bytes to convert from src_bytes
 * @param [out] dest_hex Destination buffer to receive hex encoding
 * @param dest_size_max Maximum buffer size for the hex encoded `dest_hex` buffer
 *                      including null-terminator if needed.
 * @param flags Flags from `HexFlags` for formatting options
 *
 * @return CHIP_ERROR_BUFFER_TOO_SMALL on dest_max_size too small to fit output
 * @return CHIP_ERROR_INVALID_ARGUMENT if either src_bytes or dest_hex is
 *                                     nullptr without the corresponding size
 *                                     being 0.
 * @return CHIP_NO_ERROR on success
 */

CHIP_ERROR BytesToHex(const uint8_t * src_bytes, size_t src_size, char * dest_hex, size_t dest_size_max, BitFlags<HexFlags> flags);

/**
 * Encode a uint64_t into hexadecimal, with or without null-termination
 * and using either lowercase or uppercase hex. The output will be in a big-engian
 * order.
 *
 * Default is lowercase output, not null-terminated.
 *
 * If `flags` has `HexFlags::kNullTerminate` set, treat `dest_hex` as a
 * null-terminated string buffer. The function returns CHIP_ERROR_BUFFER_TOO_SMALL
 * if `dest_size_max` can't fit the entire encoded buffer, and the
 * null-terminator if enabled. This function will never output truncated data.
 * The result either fits and is written, or does not fit and nothing is written
 * to `dest_hex`.
 *
 * On success, number of bytes written to destination is always
 *   output_size = 16 + ((flags & HexFlags::kNullTerminate) ? 1 : 0);
 *
 * @param src 64-bit number to convert
 * @param [out] dest_hex Destination buffer to receive hex encoding
 * @param dest_size_max Maximum buffer size for the hex encoded `dest_hex` buffer
 *                      including null-terminator if needed.
 * @param flags Flags from `HexFlags` for formatting options
 *
 * @return CHIP_ERROR_BUFFER_TOO_SMALL on dest_max_size too small to fit output
 * @return CHIP_ERROR_INVALID_ARGUMENT if either src_bytes or dest_hex is nullptr
 * @return CHIP_NO_ERROR on success
 */
CHIP_ERROR Uint64ToHex(uint64_t src, char * dest_hex, size_t dest_size_max, BitFlags<HexFlags> flags);

/** Same as Uint64ToHex() but for uint32_t. */
CHIP_ERROR Uint32ToHex(uint32_t src, char * dest_hex, size_t dest_size_max, BitFlags<HexFlags> flags);

/** Same as Uint64ToHex() but for uint16_t. */
CHIP_ERROR Uint16ToHex(uint16_t src, char * dest_hex, size_t dest_size_max, BitFlags<HexFlags> flags);

// Alias for Uppercase option, no null-termination
inline CHIP_ERROR BytesToUppercaseHexBuffer(const uint8_t * src_bytes, size_t src_size, char * dest_hex_buf, size_t dest_size_max)
{
    return BytesToHex(src_bytes, src_size, dest_hex_buf, dest_size_max, HexFlags::kUppercase);
}

// Alias for Lowercase option, no null-termination
inline CHIP_ERROR BytesToLowercaseHexBuffer(const uint8_t * src_bytes, size_t src_size, char * dest_hex_buf, size_t dest_size_max)
{
    return BytesToHex(src_bytes, src_size, dest_hex_buf, dest_size_max, HexFlags::kNone);
}

// Alias for Uppercase option, with null-termination
inline CHIP_ERROR BytesToUppercaseHexString(const uint8_t * src_bytes, size_t src_size, char * dest_hex_str, size_t dest_size_max)
{
    return BytesToHex(src_bytes, src_size, dest_hex_str, dest_size_max, HexFlags::kUppercaseAndNullTerminate);
}

// Alias for Lowercase option, with null-termination
inline CHIP_ERROR BytesToLowercaseHexString(const uint8_t * src_bytes, size_t src_size, char * dest_hex_str, size_t dest_size_max)
{
    return BytesToHex(src_bytes, src_size, dest_hex_str, dest_size_max, HexFlags::kNullTerminate);
}

/**
 * @brief Dumps a binary buffer to log as hexadecimal
 *
 * Output is 32 bytes per line of uppercase hex, prepended with a given label.
 *
 * This function is useful to dump binary buffers such as certificates
 * which may need to be extracted from logs during debugging.
 *
 * Format is organized to allow easy extraction by searching the regex
 * `LABEL>>>[0-9A-F]+$` where LABEL is the `label` argument passed.
 *
 * Format looks like:
 *
 * ```
 * label>>>A54A39294B28886E8BFC15B44105A3FD22745225983A753E6BB82DA7C62493BF
 * label>>>02C3ED03D41B6F7874E7E887321DE7B4872CEB9F080B6ECE14A8ABFA260573A3
 * label>>>8D759C
 * ```
 *
 * If buffer is empty, at least one line with the `LABEL>>>` will be logged,
 * with no data.
 *
 * @param label - label to prepend. If nullptr, no label will be prepended
 * @param span - Span over buffer that needs to be dumped.
 */
void LogBufferAsHex(const char * label, const ByteSpan & span);

/**
 * Convert a buffer of hexadecimal characters to bytes. Supports both lowercase
 * and uppercase (or a mix of cases) hexadecimal characters. Supported input is
 * [0-9a-fA-F]. The input is assumed to be in a big-endian order. The output is
 * also in a big-endian order.
 *
 * @param src_hex a pointer to the character buffer to convert. It is not
 *                assumed to be null-terminated.
 * @param src_size the number of characters to convert from src_hex.
 * @param dest_bytes the buffer to fill with the decoded bytes.
 * @param dest_size_max the total size of the buffer to be filled.
 *
 * @return 0 on errors:
 *           - dest_size_max not big enough.
 *           - src_size not even.
 *           - Some character not in [0-9a-fA-F] is present in src_hex.
 *         Otherwise, returns number of bytes actually decoded from the string on success.
 */
size_t HexToBytes(const char * src_hex, const size_t src_size, uint8_t * dest_bytes, size_t dest_size_max);

/**
 * Convert a buffer of hexadecimal characters into uint64_t. Supports only
 * uppercase hexadecimal input characters. Supported input is [0-9A-F].
 * The input is assumed to be in a big-endian order.
 *
 * @param src_hex a pointer to the character buffer to convert. It is not
 *                assumed to be null-terminated.
 * @param src_size the number of characters to convert from src_hex.
 * @param dest 64-bit number to output.
 *
 * @return 0 on errors:
 *           - src_size not even.
 *           - Some character not in [0-9A-F] is present in src_hex.
 *         Otherwise, returns 8 (number of bytes actually decoded from the string) on success.
 */
size_t UppercaseHexToUint64(const char * src_hex, const size_t src_size, uint64_t & dest);

/** Same as UppercaseHexToUint64() but for uint32_t. */
size_t UppercaseHexToUint32(const char * src_hex, const size_t src_size, uint32_t & dest);

/** Same as UppercaseHexToUint64() but for uint16_t. */
size_t UppercaseHexToUint16(const char * src_hex, const size_t src_size, uint16_t & dest);

/**
 * Computes the hex encoded length for a given input length.
 * Left shift to generate optimized equivalent of LEN*2.
 */
#define HEX_ENCODED_LENGTH(LEN) ((LEN) << 1)

/**
 * Computes the maximum possible decoded length for a given hex string input length.
 * Right shift to generate optimized equivalent of LEN/2.
 */
#define HEX_MAX_DECODED_LENGTH(LEN) ((LEN) >> 1)

} // namespace Encoding
} // namespace chip
