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

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/BitFlags.h>

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
 * and using either lowercase or uppercase hex.
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
 * @param src_bytes Pointer to non-null buffer to convert
 * @param src_size Number of bytes to convert from src_bytes
 * @param [out] dest_hex Destination buffer to receive hex encoding
 * @param dest_size_max Maximum buffer size for the hex encoded `dest_hex` buffer
 *                      including null-terminator if needed.
 * @param flags Flags from `HexFlags` for formatting options
 *
 * @return CHIP_ERROR_BUFFER_TOO_SMALL on dest_max_size too small to fit output
 * @return CHIP_ERROR_INVALID_ARGUMENT if either src_bytes or dest_hex is nullptr
 * @return CHIP_NO_ERROR on success
 */

CHIP_ERROR BytesToHex(const uint8_t * src_bytes, size_t src_size, char * dest_hex, size_t dest_size_max, BitFlags<HexFlags> flags);

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
 * Convert a buffer of hexadecimal characters to bytes.  Supports both lowercase
 * and uppercase (or a mix of cases) hexadecimal characters. Supported input is
 * [0-9a-fA-F].
 *
 * @param srcHex a pointer to the character buffer to convert.  It is not
 *               assumed to be null-terminated.
 * @param srcLen the length of the character buffer to convert.
 * @param destBytes the buffer to fill with the decoded bytes.
 * @param destMaxLen the total size of the buffer to be filled.
 *
 * @return 0 on errors:
 *           - destMaxLen not big enough.
 *           - srcLen not even.
 *           - Some character not in [0-9a-fA-F] is present in srcHex.
 *         number of bytes actually decoded from the string on success.
 */
size_t HexToBytes(const char * srcHex, const size_t srcLen, uint8_t * destBytes, size_t destMaxLen);

} // namespace Encoding
} // namespace chip
