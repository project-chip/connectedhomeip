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

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <support/logging/ChipLogging.h>

#include "esp_log.h"

using namespace ::chip;
using namespace ::chip::DeviceLayer::Internal;

namespace {

void GetModuleName(char * buf, uint8_t module)
{
    if (module == ::chip::Logging::kLogModule_DeviceLayer)
    {
        memcpy(buf, "DL", 3);
    }
    else
    {
        ::chip::Logging::GetModuleName(buf, module);
    }
}

} // unnamed namespace

namespace nl {
namespace chip {
namespace Logging {

void Log(uint8_t module, uint8_t category, const char * msg, ...)
{
    va_list v;

    va_start(v, msg);

    if (IsCategoryEnabled(category))
    {
        enum
        {
            kMaxTagLen = 7 + nlChipLoggingModuleNameLen
        };
        char tag[kMaxTagLen + 1];
        size_t tagLen;
        char formattedMsg[CHIP_DEVICE_CONFIG_LOG_MESSAGE_MAX_SIZE];

        strcpy(tag, "chip[");
        tagLen = strlen(tag);
        ::GetModuleName(tag + tagLen, module);
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

    va_end(v);
}

} // namespace Logging
} // namespace chip
} // namespace nl
