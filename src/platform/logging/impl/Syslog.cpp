/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <platform/logging/LogV.h>

#include <lib/core/CHIPConfig.h>
#include <lib/support/logging/Constants.h>

#include <mutex>
#include <syslog.h>

#ifndef CHIP_SYSLOG_IDENT
#define CHIP_SYSLOG_IDENT nullptr
#endif

#ifndef CHIP_SYSLOG_FACILITY
#define CHIP_SYSLOG_FACILITY LOG_DAEMON
#endif

namespace chip {
namespace Logging {
namespace Platform {

namespace {
int LogPriority(uint8_t category)
{
    switch (category)
    {
    case kLogCategory_Error:
        return LOG_ERR;
    case kLogCategory_Progress:
        return LOG_NOTICE;
    default:
        return LOG_DEBUG;
    }
}
} // namespace

void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    static std::mutex sMutex;
    static bool sInitialized = false;
    static char sBuffer[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
    std::lock_guard guard(sMutex);

    if (!sInitialized)
    {
        openlog(CHIP_SYSLOG_IDENT, LOG_CONS | LOG_PID, CHIP_SYSLOG_FACILITY);
        sInitialized = true;
    }

    // Pre-format the message so we can include the module name
    vsnprintf(sBuffer, sizeof(sBuffer), msg, v);
    syslog(LogPriority(category), "%s: %s", module, sBuffer);
}

} // namespace Platform
} // namespace Logging
} // namespace chip
