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

#include <core/CHIPError.h>
#include <stdint.h>
#include <stdlib.h>

namespace chip {
namespace Encoding {

/**
 * Encode a buffer of bytes into hexadecimal, with or without nul-termination
 * and using either lowercase or uppercase hex.
 *
 * If `nul_terminate` is true, treat `dest_hex` as a nul-terminated string buffer.
 * The function returns CHIP_ERROR_BUFFER_TOO_SMALL if dest_size_max can't fit
 * the entire encoded buffer, and the nul-terminator if enabled. This function
 * will never output truncated data. The result either fits and is written,
 * or does not fit and nothing is written to dest_hex.
 *
 * If src_bytes and dest_hex overlap, the results may be incorrect, depending
 * on overlap, but only the core validity checks are done and it's possible to
 * get CHIP_NO_ERROR with erroneous output.
 *
 * On success, number of bytes written to destination is always:
 *   output_size = (src_size * 2) + (nul_terminate ? 1 : 0);
 *
 * @param src_bytes Pointer to non-null buffer to convert
 * @param src_size Number of bytes to convert from src_bytes
 * @param [out] dest_hex Destination buffer to receive hex encoding
 * @param dest_size_max Maximum buffer size for the hex encoded `dest_hex` buffer
 *                      including nul-terminator if needed.
 * @param uppercase Use upper-case hex if true, otherwise lower-case hex
 * @param nul_terminate Add nul terminator if true. See description
 *
 * @return CHIP_ERROR_BUFFER_TOO_SMALL on dest_max_size too small to fit output
 * @return CHIP_ERROR_INVALID_ARGUMENT if either src_bytes or dest_hex is nullptr
 * @return CHIP_NO_ERROR on success
 */

CHIP_ERROR BytesToHex(const uint8_t * src_bytes, size_t src_size, char * dest_hex, size_t dest_size_max, bool uppercase,
                      bool nul_terminate);

} // namespace Encoding
} // namespace chip
