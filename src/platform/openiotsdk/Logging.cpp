/*
 *
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

/**
 *    @file
 *          Provides an implementation of logging support
 *          for Open IOT SDK platform.
 */

#include <lib/core/CHIPConfig.h>
#include <lib/support/EnforceFormat.h>
#include <lib/support/logging/CHIPLogging.h>
#include <lib/support/logging/Constants.h>
#include <platform/logging/LogV.h>

#include <stdio.h>
#include <string.h>

namespace chip {
namespace DeviceLayer {

/**
 * Called whenever a log message is emitted by chip or LwIP.
 *
 * This function is intended be overridden by the application to, e.g.,
 * schedule output of queued log entries.
 */
void __attribute__((weak)) OnLogOutput() {}

} // namespace DeviceLayer

namespace Logging {
namespace Platform {

/**
 * CHIP log output functions.
 */
void ENFORCE_FORMAT(3, 0) LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    char logMsgBuffer[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];

    const char * category_prefix;
    switch (category)
    {
    case kLogCategory_Error:
        category_prefix = "\x1b[31m[ERR]\x1b[0m";
        break;
    case kLogCategory_Progress:
    default:
        category_prefix = "\x1b[32m[INF]\x1b[0m";
        break;
    case kLogCategory_Detail:
        category_prefix = "\x1b[90m[DBG]\x1b[0m";
        break;
    case kLogCategory_Automation:
        category_prefix = "";
        break;
    }

    int header_len  = snprintf(logMsgBuffer, sizeof(logMsgBuffer), "%s [%s] ", category_prefix, module);
    int content_len = vsnprintf(logMsgBuffer + header_len, sizeof(logMsgBuffer) - header_len, msg, v);
    int trailer_len = snprintf(logMsgBuffer + header_len + content_len, sizeof(logMsgBuffer) - header_len - content_len, "\r\n");
    fwrite(logMsgBuffer, header_len + content_len + trailer_len, 1, stdout);

    // Let the application know that a log message has been emitted.
    DeviceLayer::OnLogOutput();
}

void ois_logging_init()
{
#if defined(NDEBUG) && CHIP_CONFIG_TEST == 0
    SetLogFilter(LogCategory::kLogCategory_Progress);
#endif
}

} // namespace Platform
} // namespace Logging
} // namespace chip
