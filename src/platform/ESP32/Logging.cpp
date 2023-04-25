/* See Project CHIP LICENSE file for licensing information. */

#include <platform/logging/LogV.h>

#include <lib/core/CHIPConfig.h>
#include <lib/support/EnforceFormat.h>
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

void ENFORCE_FORMAT(3, 0) LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    char tag[11];

    snprintf(tag, sizeof(tag), "chip[%s]", module);
    tag[sizeof(tag) - 1] = 0;

    switch (category)
    {
    case kLogCategory_Error:
        if (esp_log_default_level >= ESP_LOG_ERROR)
        {
            printf(LOG_COLOR_E "E");                        // set color
            printf(" (%u) %s: ", esp_log_timestamp(), tag); // add timestamp
            esp_log_writev(ESP_LOG_ERROR, tag, msg, v);
            printf(LOG_RESET_COLOR "\n");
        }
        break;
    case kLogCategory_Progress:
    default:
        if (esp_log_default_level >= ESP_LOG_INFO)
        {
            printf(LOG_COLOR_I "I");                        // set color
            printf(" (%u) %s: ", esp_log_timestamp(), tag); // add timestamp
            esp_log_writev(ESP_LOG_INFO, tag, msg, v);
            printf(LOG_RESET_COLOR "\n");
        }
        break;
    case kLogCategory_Detail:
        if (esp_log_default_level >= ESP_LOG_DEBUG)
        {
            printf(LOG_COLOR_D "D");                        // set color
            printf(" (%u) %s: ", esp_log_timestamp(), tag); // add timestamp
            esp_log_writev(ESP_LOG_DEBUG, tag, msg, v);
            printf(LOG_RESET_COLOR "\n");
        }
        break;
    }
}

} // namespace Platform
} // namespace Logging
} // namespace chip
