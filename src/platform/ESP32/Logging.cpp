/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
 *    All rights reserved.
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
            kMaxTagLen = 7 + ChipLoggingModuleNameLen
        };
        char tag[kMaxTagLen + 1];
        size_t tagLen;
        char formattedMsg[CHIP_DEVICE_CONFIG_LOG_MESSAGE_MAX_SIZE];

        strcpy(tag, "chip[");
        tagLen = strlen(tag);
        GetModuleName(tag + tagLen, module);
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
        case kLogCategory_Retain:
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
