/* See Project chip LICENSE file for licensing information. */

#include <lib/support/logging/Constants.h>
#include <platform/logging/LogV.h>

#include <lib/core/CHIPConfig.h>
#include <lib/support/logging/Constants.h>

#include <os/log.h>

#include <inttypes.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

namespace chip {
namespace Logging {
namespace Platform {

void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    timeval time;
    gettimeofday(&time, NULL);
    long ms = (time.tv_sec * 1000) + (time.tv_usec / 1000);

    uint64_t ktid;
    pthread_threadid_np(NULL, &ktid);

    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
    int32_t prefixLen   = snprintf(formattedMsg, sizeof(formattedMsg), "[%ld] [%lld:%lld] CHIP: [%s] ", ms, (long long) getpid(),
                                 (long long) ktid, module);
    static os_log_t log = os_log_create("com.zigbee.chip", "all");
    if (prefixLen < 0)
    {
        // This should not happen
        return;
    }

    if (static_cast<size_t>(prefixLen) >= sizeof(formattedMsg))
    {
        prefixLen = sizeof(formattedMsg) - 1;
    }

    vsnprintf(formattedMsg + prefixLen, sizeof(formattedMsg) - static_cast<size_t>(prefixLen), msg, v);

    switch (category)
    {
    case kLogCategory_Error:
        os_log_with_type(log, OS_LOG_TYPE_ERROR, "🔴 %{public}s", formattedMsg);
#if TARGET_OS_MAC && TARGET_OS_IPHONE == 0
        fprintf(stdout, "\033[1;31m");
#endif
        break;

    case kLogCategory_Progress:
        os_log_with_type(log, OS_LOG_TYPE_DEFAULT, "🔵 %{public}s", formattedMsg);
#if TARGET_OS_MAC && TARGET_OS_IPHONE == 0
        fprintf(stdout, "\033[0;32m");
#endif
        break;

    case kLogCategory_Detail:
        os_log_with_type(log, OS_LOG_TYPE_DEFAULT, "🟢 %{public}s", formattedMsg);
#if TARGET_OS_MAC && TARGET_OS_IPHONE == 0
        fprintf(stdout, "\033[0;34m");
#endif
        break;
    }
#if TARGET_OS_MAC && TARGET_OS_IPHONE == 0
    fprintf(stdout, "%s\033[0m\n", formattedMsg);
#endif
}

} // namespace Platform
} // namespace Logging
} // namespace chip
