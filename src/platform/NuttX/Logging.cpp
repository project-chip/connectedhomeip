/* See Project CHIP LICENSE file for licensing information. */

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
           static_cast<long long>(getpid()), static_cast<long long>(gettid()), module);
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
