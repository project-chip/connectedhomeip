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

    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
    vsnprintf(formattedMsg, sizeof(formattedMsg), msg, v);

    switch (category)
    {
    case kLogCategory_Error:
        ESP_LOGE(tag, "%s", formattedMsg);
        break;
    case kLogCategory_Progress:
    default:
        ESP_LOGI(tag, "%s", formattedMsg);
        break;
    case kLogCategory_Detail:
        ESP_LOGV(tag, "%s", formattedMsg);
        break;
    }
}

} // namespace Platform
} // namespace Logging
} // namespace chip
