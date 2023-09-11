/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ZclString.h"

namespace chip {

// ZCL strings are stored as pascal-strings (first byte contains the length of
// the data), and a length of 255 means "invalid string" so the maximum actually
// allowed string length is 254.
constexpr size_t kBufferMaximumSize = 254;

CHIP_ERROR MakeZclCharString(MutableByteSpan & buffer, const char * cString)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (buffer.size() == 0)
    {
        // We can't even put a 0 length in there.
        return CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG;
    }
    size_t len              = strlen(cString);
    size_t availableStorage = min(buffer.size() - 1, kBufferMaximumSize);
    if (len > availableStorage)
    {
        buffer.data()[0] = 0;
        return CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG;
    }

    buffer.data()[0] = static_cast<uint8_t>(len);
    memcpy(&buffer.data()[1], cString, len);
    return err;
}

} // namespace chip
