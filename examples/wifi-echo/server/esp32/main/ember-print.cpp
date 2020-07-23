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
    int len = strlen(format);
    if (format != NULL && len > 0)
    {
        char formatLn[len + 2];
        strncpy(formatLn, format, len);
        formatLn[len]     = '\n';
        formatLn[len + 1] = 0;

        va_list args;
        va_start(args, format);
        chip::Logging::LogV(chip::Logging::kLogModule_Zcl, chip::Logging::kLogCategory_Progress, formatLn, args);
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
        for (uint32_t dst_idx = 0, index = 0; dst_idx < outStringLength && index < length; dst_idx += perByteCharCount, index++)
        {

            snprintf(result + dst_idx, perByteCharCount + 1, perByteFormatStr, buffer[index]);
        }
        result[outStringLength] = 0;
        emberAfPrint(category, "%s", result);
    }
    else
    {
        emberAfPrint(EMBER_AF_PRINT_CORE, "NULL");
    }
}
}
