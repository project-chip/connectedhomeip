/* See Project CHIP LICENSE file for licensing information. */

#include <platform/logging/LogV.h>

#include <lib/core/CHIPConfig.h>
#include <lib/support/logging/Constants.h>

#include <stdio.h>

// #ifdef LOG_LOCAL_LEVEL
// #undef LOG_LOCAL_LEVEL
// #endif
// #define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

extern "C" {
#include <blog.h>
}

#ifdef PW_RPC_ENABLED
#include "PigweedLogger.h"
#endif

namespace chip {
namespace Logging {
namespace Platform {

void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];

#ifndef PW_RPC_ENABLED
    vsnprintf(formattedMsg, sizeof(formattedMsg), msg, v);

    switch (category)
    {
    case kLogCategory_Error:
        log_error("[%s] %s\r\n", module, formattedMsg);
        break;
    case kLogCategory_Progress:
    default:
        log_info("[%s] %s\r\n", module, formattedMsg);
        break;
    case kLogCategory_Detail:
        log_trace("[%s] %s\r\n", module, formattedMsg);
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
