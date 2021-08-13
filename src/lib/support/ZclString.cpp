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

constexpr int kBufferMaximumSize = 254;

CHIP_ERROR MakeZclCharString(chip::MutableByteSpan & buffer, const char * cString)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    size_t len = strlen(cString);
    if (strlen(cString) > buffer.size())
    {
        err = CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG;
        // set pascal string length to a maximum of kBufferMaximumSize
        len = buffer.size() > kBufferMaximumSize ? kBufferMaximumSize : buffer.size();
    }

    buffer.data()[0] = static_cast<uint8_t>(len);
    memcpy(&buffer.data()[1], cString, len);
    return err;
}

} // namespace chip
