/*
 *    Copyright (c) 2020-2023 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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

#include "BinaryLogging.h"

namespace chip {
namespace Logging {

#if _CHIP_USE_LOGGING

void LogByteSpan(uint8_t module, uint8_t category, const ByteSpan & span)
{
    // Maximum number of characters needed to print 8 byte buffer including formatting (0x)
    // 8 bytes * (2 nibbles per byte + 4 character for ", 0x") + null termination.
    // Rounding up to 50 bytes.
    char output[50];
    size_t offset = 0;
    for (unsigned int i = 0; i < span.size(); i++)
    {
        if (i % 8 == 0 && offset != 0)
        {
            Log(module, category, "%s", output);
            offset = 0;
        }
        int result = snprintf(&output[offset], sizeof(output) - offset, "0x%02x, ", (unsigned char) span.data()[i]);
        if (result > 0)
        {
            offset += static_cast<size_t>(result);
        }
        else
        {
            Log(module, Logging::kLogCategory_Error, "Failed to print ByteSpan buffer");
            return;
        }
    }
    if (offset != 0)
    {
        Log(module, category, "%s", output);
    }
}

#endif // _CHIP_USE_LOGGING

} // namespace Logging
} // namespace chip
