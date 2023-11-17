/*
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <stdio.h>

#include <platform/CHIPDeviceConfig.h>
#include <platform/logging/LogV.h>

#include <CHIPDevicePlatformConfig.h>
#include <lib/core/CHIPConfig.h>
#include <lib/support/logging/Constants.h>

#if PW_RPC_ENABLED
#include "PigweedLogger.h"
#endif

#include <FreeRTOS.h>
#include <task.h>
#if CHIP_DEVICE_LAYER_TARGET_BL602 || CHIP_DEVICE_LAYER_TARGET_BL702 || CHIP_DEVICE_LAYER_TARGET_BL702L
#include <utils_log.h>
#endif

namespace chip {
namespace Logging {
namespace Platform {

static char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
#if !PW_RPC_ENABLED
    int lmsg = 0;

    vsnprintf(formattedMsg, sizeof(formattedMsg), msg, v);

    lmsg = strlen(formattedMsg);
    if (lmsg)
    {
        /** remove duplicate \r\n */
        if (lmsg >= 2 && formattedMsg[lmsg - 2] == '\r' && formattedMsg[lmsg - 1] == '\n')
        {
            lmsg -= 2;
        }
        else if (lmsg >= 1 && formattedMsg[lmsg - 1] == '\n')
        {
            lmsg -= 1;
        }

        formattedMsg[lmsg] = '\0';
    }

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

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
extern "C" void otPlatLog(int aLogLevel, int aLogRegion, const char * aFormat, ...)
{
    va_list v;
    uint8_t category = chip::Logging::kLogCategory_Error;

    (void) aLogRegion;

    if (aLogLevel == 1 && aLogLevel == 2)
    {
        category = chip::Logging::kLogCategory_Error;
    }
    else
    {
        category = chip::Logging::kLogCategory_Progress;
    }

    va_start(v, aFormat);

    chip::Logging::Platform::LogV("OTBR", category, aFormat, v);

    va_end(v);
}
#endif

#if CHIP_SYSTEM_CONFIG_USE_LWIP
/**
 * LwIP log output function.
 */
extern "C" void LwIPLog(const char * msg, ...)
{
    va_list v;
    uint8_t category = chip::Logging::kLogCategory_Error;

    va_start(v, msg);

    chip::Logging::Platform::LogV("LWIP", category, msg, v);

    va_end(v);
}
#endif // #if CHIP_SYSTEM_CONFIG_USE_LWIP
