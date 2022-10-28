/* See Project chip LICENSE file for licensing information. */

#include <lib/support/EnforceFormat.h>
#include <lib/support/logging/Constants.h>
#include <platform/logging/LogV.h>
#include <platform/Darwin/Logging.h>

#include <lib/core/CHIPConfig.h>

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

void ENFORCE_FORMAT(3, 0) getDarwinLogMessageFormat(const char * module, uint8_t category, const char * msg, 
        va_list v, uint16_t size, char * formattedMsg)
{
    timeval time;
    gettimeofday(&time, nullptr);
    long ms = (time.tv_sec * 1000) + (time.tv_usec / 1000);

    uint64_t ktid;
    pthread_threadid_np(nullptr, &ktid);

    int32_t prefixLen = snprintf(formattedMsg, size, "[%ld] [%lld:%lld] CHIP: [%s] ", ms, (long long) getpid(),
                                 (long long) ktid, module);

    if (prefixLen < 0)
    {
        // In the event that we could not format the message, simply copy over the
        // unformatted message. It won't contain all the information that we need, but
        // it's better than dropping a log on the floor.
        strncpy(formattedMsg, msg, size);
        return;
    }

    if (static_cast<size_t>(prefixLen) >= size)
    {
        prefixLen = size - 1;
    }

    vsnprintf(formattedMsg + prefixLen, size - static_cast<size_t>(prefixLen), msg, v);

    // Explicitly NULL-terminate the formatted string for safety.
    formattedMsg[size - 1] = '\0';
}

void ENFORCE_FORMAT(3, 0) LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
    getDarwinLogMessageFormat(module, category, msg, v, CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE, formattedMsg);

    static os_log_t log = os_log_create("com.csa.matter", "all");

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
