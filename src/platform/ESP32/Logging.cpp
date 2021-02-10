/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *          Provides implementations for the CHIP logging functions
 *          on the ESP32 platform.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <support/logging/CHIPLogging.h>

#ifdef LOG_LOCAL_LEVEL
#undef LOG_LOCAL_LEVEL
#endif
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include "esp_log.h"

using namespace ::chip;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace Logging {

void LogV(uint8_t module, uint8_t category, const char * msg, va_list v)
{
    if (IsCategoryEnabled(category))
    {
        enum
        {
            kMaxTagLen = 7 + chip::Logging::kMaxModuleNameLen
        };
        char tag[kMaxTagLen + 1];
        size_t tagLen;
        char formattedMsg[CHIP_DEVICE_CONFIG_LOG_MESSAGE_MAX_SIZE];

        strcpy(tag, "chip[");
        tagLen = strlen(tag);
        GetModuleName(tag + tagLen, chip::Logging::kMaxModuleNameLen + 1, module);
        tagLen        = strlen(tag);
        tag[tagLen++] = ']';
        tag[tagLen]   = 0;

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
}

} // namespace Logging

} // namespace chip
