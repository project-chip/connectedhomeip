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

#include "ZclString.h"

namespace chip {

// ZCL strings are stored as pascal-strings (first byte contains the length of the
// data), so the maximum string length is the maximum of uint8_t
constexpr size_t kBufferMaximumSize = std::numeric_limits<uint8_t>::max();

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
