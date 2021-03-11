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

#include "BytesToHex.h"

namespace chip {
namespace Encoding {

namespace {

char NibbleToHex(uint8_t nibble, bool uppercase)
{
    char x = static_cast<char>(nibble & 0xFu);

    if (x >= 10)
    {
        return static_cast<char>((x - 10) + (uppercase ? 'A' : 'a'));
    }
    else
    {
        return static_cast<char>(x + '0');
    }
}

} // namespace

CHIP_ERROR BytesToHex(const uint8_t * src_bytes, size_t src_size, char * dest_hex, size_t dest_size_max, HexFlags flags)
{
    if ((src_bytes == nullptr) || (dest_hex == nullptr))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    else if (src_size > ((SIZE_MAX - 1) / 2u))
    {
        // Output would overflow a size_t, let's bail out to avoid computation wraparounds below.
        // This condition will hit with slightly less than the very max, but is unlikely to
        // ever happen unless an error occurs and won't happen on embedded targets.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    bool nul_terminate          = (static_cast<int>(flags) & static_cast<int>(HexFlags::kNullTerminate)) != 0;
    size_t expected_output_size = (src_size * 2u) + (nul_terminate ? 1u : 0u);
    if (dest_size_max < expected_output_size)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    bool uppercase = (static_cast<int>(flags) & static_cast<int>(HexFlags::kUppercase)) != 0;
    char * cursor  = dest_hex;
    for (size_t byte_idx = 0; byte_idx < src_size; ++byte_idx)
    {
        *cursor++ = NibbleToHex((src_bytes[byte_idx] >> 4) & 0xFu, uppercase);
        *cursor++ = NibbleToHex((src_bytes[byte_idx] >> 0) & 0xFu, uppercase);
    }

    if (nul_terminate)
    {
        *cursor = '\0';
    }

    return CHIP_NO_ERROR;
}

} // namespace Encoding
} // namespace chip
