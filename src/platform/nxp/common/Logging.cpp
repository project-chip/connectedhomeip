/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Nest Labs, Inc.
 *    Copyright 2023 NXP
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
#include <lib/support/logging/CHIPLogging.h>
#include <platform/logging/LogV.h>

#include "fsl_debug_console.h"
#include <lib/core/CHIPConfig.h>
#include <platform/logging/LogV.h>

#include <lib/support/logging/Constants.h>
#include <stdio.h>

#include <FreeRTOS.h>
#include <assert.h>
#include <semphr.h>
#include <timers.h>

namespace chip {
namespace Logging {
namespace Platform {

/**
 * CHIP log output function.
 * Called only by ember-print (zcl log)
 */
void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE] = { 0 };
    size_t prefixLen;

    /* First add the timestamp to the log message */
    snprintf(formattedMsg, sizeof(formattedMsg), "[%ld] ", xTaskGetTickCount());
    prefixLen = strlen(formattedMsg);

    /* Then add log category */
    switch (category)
    {
    case kLogCategory_Error:
        snprintf(formattedMsg + prefixLen, sizeof(formattedMsg) - prefixLen, "%s ", "[ERR]");
        break;

    case kLogCategory_Progress:
        snprintf(formattedMsg + prefixLen, sizeof(formattedMsg) - prefixLen, "%s ", "[INFO]");
        break;

    default:
        snprintf(formattedMsg + prefixLen, sizeof(formattedMsg) - prefixLen, "%s ", "[TRACE]");
        break;
    }

    prefixLen = strlen(formattedMsg);

    /* Add the module to the log output */
    snprintf(formattedMsg + prefixLen, sizeof(formattedMsg) - prefixLen, "[%s] ", module);
    prefixLen = strlen(formattedMsg);

    // Append the log message.
    vsnprintf(formattedMsg + prefixLen, sizeof(formattedMsg) - prefixLen, msg, v);
    prefixLen = strlen(formattedMsg);

    /* Add CR+LF */
    snprintf(formattedMsg + prefixLen, sizeof(formattedMsg) - prefixLen, "%s", "\r\n");

    PRINTF("%s", formattedMsg);
}

/**
 * LwIP log output function.
 */
extern "C" void LwIPLog(const char * msg, ...)
{
    va_list v;

    va_start(v, msg);
    LogV("lwip", kLogCategory_Progress, msg, v);
    va_end(v);
}

} // namespace Platform
} // namespace Logging
} // namespace chip
