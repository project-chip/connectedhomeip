/*
 *
 *    Copyright (c) 2021-2025 Project CHIP Authors
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
#include <platform/logging/LogV.h>

#include <lib/core/CHIPConfig.h>
#include <lib/support/logging/Constants.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>

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
void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    struct timeval tv;

    // Should not fail per man page of gettimeofday(), but failed to get time is not a fatal error in log. The bad time value will
    // indicate the error occurred during getting time.
    gettimeofday(&tv, nullptr);

    // Lock standard output, so a single log line will not be corrupted in case
    // where multiple threads are using logging subsystem at the same time.
    flockfile(stdout);

    printf("[%" PRIu64 ".%06" PRIu64 "][%lld:%lld] CHIP:%s: ", static_cast<uint64_t>(tv.tv_sec), static_cast<uint64_t>(tv.tv_usec),
           static_cast<long long>(syscall(SYS_getpid)), static_cast<long long>(syscall(SYS_gettid)), module);
    vprintf(msg, v);
    printf("\n");
    fflush(stdout);

    funlockfile(stdout);

    // Let the application know that a log message has been emitted.
    DeviceLayer::OnLogOutput();
}

} // namespace Platform
} // namespace Logging
} // namespace chip
