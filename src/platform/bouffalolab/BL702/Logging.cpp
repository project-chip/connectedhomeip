/* See Project CHIP LICENSE file for licensing information. */

#include <platform/logging/LogV.h>

#include <lib/core/CHIPConfig.h>
#include <lib/support/logging/Constants.h>

#include <stdio.h>

extern "C" {
#include <blog.h>
}

#ifdef PW_RPC_ENABLED
#include "PigweedLogger.h"
#endif

namespace chip {
namespace Logging {
namespace Platform {

char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
#ifndef PW_RPC_ENABLED
    vsnprintf(formattedMsg, sizeof(formattedMsg), msg, v);

    switch (category)
    {
    case kLogCategory_Error:
        __utils_printf("[%10u][%s][ERROR] %s\r\n", xTaskGetTickCount(), module, formattedMsg);
        break;
    case kLogCategory_Progress:
        __utils_printf("[%10u][%s][PROGR] %s\r\n", xTaskGetTickCount(), module, formattedMsg);
        break;
    case kLogCategory_Detail:
        __utils_printf("[%10u][%s][DETAIL] %s\r\n", xTaskGetTickCount(), module, formattedMsg);
        break;
    }
#else
    size_t prefixLen = 0;

    switch (category)
    {
    case kLogCategory_Error:
        strcpy(formattedMsg, "[ERROR] ");
        break;
    case kLogCategory_Progress:
    default:
        strcpy(formattedMsg, "[INFO] ");
        break;
    case kLogCategory_Detail:
        strcpy(formattedMsg, "[TRACE] ");
        break;
    }

    prefixLen = strlen(formattedMsg);
    snprintf(formattedMsg + prefixLen, sizeof(formattedMsg) - prefixLen, "[%s] ", module);
    prefixLen = strlen(formattedMsg);
    vsnprintf(formattedMsg + prefixLen, sizeof(formattedMsg) - prefixLen, msg, v);

    PigweedLogger::putString(formattedMsg, strlen(formattedMsg));
    const char * newline = "\r\n";
    PigweedLogger::putString(newline, strlen(newline));
#endif
}

} // namespace Platform
} // namespace Logging
} // namespace chip
