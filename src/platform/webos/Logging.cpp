/*
 *
 *    Copyright (c) 2021-2026 Project CHIP Authors
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
 *          Platform-specific implementation of KVS for webOS.
 */
#include <lib/core/CHIPConfig.h>
#include <lib/support/EnforceFormat.h>
#include <lib/support/logging/Constants.h>
#include <platform/logging/LogV.h>

#include <lib/core/CHIPConfig.h>
#include <lib/support/logging/Constants.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>

#if CHIP_USE_PW_LOGGING
#include <pw_log/log.h>
#endif // CHIP_USE_PW_LOGGING

#ifdef USE_SYSLOG
#include <syslog.h>
#elif USE_PMLOG
#include <PmLogLib.h>

#define CHIP_CORE_LOG_CONTEXT "chip-core"
#endif
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

#if USE_PMLOG
PmLogContext getPmLogContext()
{
    static PmLogContext logContext = 0;
    if (0 == logContext) {
        PmLogGetContext(CHIP_CORE_LOG_CONTEXT, &logContext);
    }
    return logContext;
}
#endif

/**
 * CHIP log output functions.
 */
void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    struct timeval tv;

    // Should not fail per man page of gettimeofday(), but failed to get time is not a fatal error in log. The bad time value will
    // indicate the error occurred during getting time.
    gettimeofday(&tv, nullptr);

#if !CHIP_USE_PW_LOGGING
#ifdef USE_SYSLOG
    vsyslog(category, msg, v);
#elif USE_PMLOG
    char buffer[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
    vsnprintf(buffer, sizeof(buffer), msg, v);
    buffer[sizeof(buffer) - 1] = 0;

    switch (static_cast<LogCategory>(category))
    {
    case kLogCategory_Error:
        PmLogError(getPmLogContext(), module, 0, "%s", buffer);
        break;
    case kLogCategory_Progress:
        PmLogInfo(getPmLogContext(), module, 0, "%s", buffer);
        break;
    case kLogCategory_Detail:
    case kLogCategory_None:
    case kLogCategory_Automation:
        PmLogDebug(getPmLogContext(), "%s %s", module, buffer);
        break;
    }
#else
    // Lock standard output, so a single log line will not be corrupted in case
    // where multiple threads are using logging subsystem at the same time.
    flockfile(stdout);

    printf("[%" PRIu64 ".%06" PRIu64 "][%lld:%lld] CHIP:%s: ", static_cast<uint64_t>(tv.tv_sec), static_cast<uint64_t>(tv.tv_usec),
           static_cast<long long>(syscall(SYS_getpid)), static_cast<long long>(syscall(SYS_gettid)), module);
    vprintf(msg, v);
    printf("\n");
    fflush(stdout);

    funlockfile(stdout);

#endif
#else  // !CHIP_USE_PW_LOGGING
    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
    snprintf(formattedMsg, sizeof(formattedMsg),
             "[%" PRIu64 ".%06" PRIu64 "][%lld:%lld] CHIP:%s: ", static_cast<uint64_t>(tv.tv_sec),
             static_cast<uint64_t>(tv.tv_usec), static_cast<long long>(syscall(SYS_getpid)),
             static_cast<long long>(syscall(SYS_gettid)), module);
    size_t len = strnlen(formattedMsg, sizeof(formattedMsg));
    vsnprintf(formattedMsg + len, sizeof(formattedMsg) - len, msg, v);

    switch (static_cast<LogCategory>(category))
    {
    case kLogCategory_Error:
        PW_LOG_ERROR("%s", formattedMsg);
        break;
    case kLogCategory_Progress:
        PW_LOG_INFO("%s", formattedMsg);
        break;
    case kLogCategory_Detail:
    case kLogCategory_None:
    case kLogCategory_Automation:
        PW_LOG_DEBUG("%s", formattedMsg);
        break;
    }
#endif // !CHIP_USE_PW_LOGGING


    // Let the application know that a log message has been emitted.
    DeviceLayer::OnLogOutput();
}

} // namespace Platform
} // namespace Logging
} // namespace chip
