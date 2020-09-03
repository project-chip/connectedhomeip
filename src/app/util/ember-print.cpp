/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "gen/gen_config.h"

#include <support/CHIPLogging.h>

bool emberAfPrintReceivedMessages = true;

using namespace chip::Logging;
extern "C" {

void emberAfPrint(int category, const char * format, ...)
{
    if (format != NULL)
    {
        va_list args;
        va_start(args, format);
        chip::Logging::LogV(chip::Logging::kLogModule_Zcl, chip::Logging::kLogCategory_Progress, format, args);
        va_end(args);
    }
}

void emberAfPrintln(int category, const char * format, ...)
{
    if (format != NULL)
    {
        va_list args;
        va_start(args, format);
        chip::Logging::LogV(chip::Logging::kLogModule_Zcl, chip::Logging::kLogCategory_Progress, format, args);
        va_end(args);
    }
}

void emberAfPrintBuffer(int category, const uint8_t * buffer, uint16_t length, bool withSpace)
{
    if (buffer != NULL && length > 0)
    {
        const char * perByteFormatStr = withSpace ? "%02hhX " : "%02hhX";
        uint8_t perByteCharCount      = withSpace ? 3 : 2;

        uint32_t outStringLength = length * perByteCharCount + 1;
        char result[outStringLength];
        for (uint32_t dst_idx = 0, index = 0; dst_idx < outStringLength - 1 && index < length; dst_idx += perByteCharCount, index++)
        {

            snprintf(result + dst_idx, outStringLength - dst_idx, perByteFormatStr, buffer[index]);
        }
        result[outStringLength - 1] = 0;
        emberAfPrint(category, "%s", result);
    }
    else
    {
        emberAfPrint(EMBER_AF_PRINT_CORE, "NULL");
    }
}

void emberAfPrintString(int category, const uint8_t * string)
{
    emberAfPrint(category, "%s", string);
}
}
