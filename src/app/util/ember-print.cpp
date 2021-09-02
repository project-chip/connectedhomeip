/**
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <app/util/af.h>
#include <app/util/debug-printing.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceConfig.h>

bool emberAfPrintReceivedMessages = true;

using namespace chip::Logging;

void emberAfPrint(int category, const char * format, ...)
{
#if _CHIP_USE_LOGGING
    if (format != nullptr)
    {
        va_list args;
        va_start(args, format);
        chip::Logging::LogV(chip::Logging::kLogModule_Zcl, chip::Logging::kLogCategory_Progress, format, args);
        va_end(args);
    }
#endif
}

void emberAfPrintln(int category, const char * format, ...)
{
#if _CHIP_USE_LOGGING
    if (format != nullptr)
    {
        va_list args;
        va_start(args, format);
        chip::Logging::LogV(chip::Logging::kLogModule_Zcl, chip::Logging::kLogCategory_Progress, format, args);
        va_end(args);
    }
#endif
}

// TODO: add unit tests.

void emberAfPrintBuffer(int category, const uint8_t * buffer, uint16_t length, bool withSpace)
{
    if (buffer != nullptr && length > 0)
    {
        constexpr uint16_t kBufferSize = CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE;
        const char * perByteFormatStr  = withSpace ? "%02X " : "%02X";
        const uint8_t perByteCharCount = withSpace ? 3 : 2;
        const uint16_t bytesPerBuffer  = static_cast<uint16_t>((kBufferSize - 1) / perByteCharCount);
        char result[kBufferSize];

        uint16_t index = 0;
        while (index < length)
        {
            const uint16_t remainingBytes = static_cast<uint16_t>(length - index);
            const uint16_t segmentLength  = chip::min(bytesPerBuffer, remainingBytes);
            const uint16_t segmentEnd     = static_cast<uint16_t>(index + segmentLength);
            const uint32_t outStringEnd   = segmentLength * perByteCharCount;
            for (uint32_t dst_idx = 0; dst_idx < outStringEnd && index < segmentEnd; dst_idx += perByteCharCount, index++)
            {
                snprintf(result + dst_idx, outStringEnd - dst_idx + 1, perByteFormatStr, buffer[index]);
            }
            result[outStringEnd] = 0;
            emberAfPrint(category, "%s", result);
        }
    }
    else
    {
        emberAfPrint(EMBER_AF_PRINT_CORE, "NULL");
    }
}

void emberAfPrintString(int category, const uint8_t * string)
{
    emberAfPrint(category, "%.*s", emberAfStringLength(string), string + 1);
}
