/*
 *   Copyright (c) 2025 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>

/**
 * Utility for converting a hex string to bytes, with the right error checking
 * and allocation size computation.
 *
 * Takes a functor to allocate the buffer to use for the hex bytes.  The functor
 * is expected to return uint8_t *.  The caller is responsible for cleaning up
 * this buffer as needed.
 *
 * On success, *octetCount is filled with the number of octets placed in the
 * buffer.  On failure, the value of *octetCount is undefined.
 */
template <typename F>
CHIP_ERROR HexToBytes(chip::CharSpan hex, F bufferAllocator, size_t * octetCount)
{
    *octetCount = 0;

    if (hex.size() % 2 != 0)
    {
        ChipLogError(NotSpecified, "Error while encoding '%.*s' as an octet string: Odd number of characters.",
                     static_cast<int>(hex.size()), hex.data());
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }

    const size_t bufferSize = hex.size() / 2;
    uint8_t * buffer        = bufferAllocator(bufferSize);
    if (buffer == nullptr && bufferSize != 0)
    {
        ChipLogError(NotSpecified, "Failed to allocate buffer of size: %llu", static_cast<unsigned long long>(bufferSize));
        return CHIP_ERROR_NO_MEMORY;
    }

    size_t byteCount = chip::Encoding::HexToBytes(hex.data(), hex.size(), buffer, bufferSize);
    if (byteCount == 0 && hex.size() != 0)
    {
        ChipLogError(NotSpecified, "Error while encoding '%.*s' as an octet string.", static_cast<int>(hex.size()), hex.data());
        return CHIP_ERROR_INTERNAL;
    }

    *octetCount = byteCount;
    return CHIP_NO_ERROR;
}
