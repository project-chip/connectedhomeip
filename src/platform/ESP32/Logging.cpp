/* See Project CHIP LICENSE file for licensing information. */

#include <platform/logging/LogV.h>

#include <lib/core/CHIPConfig.h>
#include <lib/support/logging/Constants.h>

#include <stdio.h>

#ifdef LOG_LOCAL_LEVEL
#undef LOG_LOCAL_LEVEL
#endif
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include "esp_log.h"

namespace chip {
namespace Logging {
namespace Platform {

void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    char tag[11];

    snprintf(tag, sizeof(tag), "chip[%s]", module);
    tag[sizeof(tag) - 1] = 0;

    switch (category)
    {
    case kLogCategory_Error: {
        {
            printf(LOG_COLOR_E "E (%" PRIu32 ") %s: ", esp_log_timestamp(), tag);
            esp_log_writev(ESP_LOG_ERROR, tag, msg, v);
            printf(LOG_RESET_COLOR "\n");
        }
    }
    break;

    case kLogCategory_Progress:
    default: {
        {
            printf(LOG_COLOR_I "I (%" PRIu32 ") %s: ", esp_log_timestamp(), tag);
            esp_log_writev(ESP_LOG_INFO, tag, msg, v);
            printf(LOG_RESET_COLOR "\n");
        }
    }
    break;

    case kLogCategory_Detail: {
        {
            printf(LOG_COLOR_D "D (%" PRIu32 ") %s: ", esp_log_timestamp(), tag);
            esp_log_writev(ESP_LOG_DEBUG, tag, msg, v);
            printf(LOG_RESET_COLOR "\n");
        }
    }
    break;
    }
}

} // namespace Platform
} // namespace Logging
} // namespace chip
