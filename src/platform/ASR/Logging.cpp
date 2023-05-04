#include <lib/support/logging/CHIPLogging.h>
#include <platform/logging/LogV.h>
#include <stdio.h>
#if (CFG_EASY_LOG_ENABLE == 1)
#include "elog.h"
#if (CFG_EASY_LOG_MODULE_EN == 1)
#include "elog_module.h"
#endif
#endif

namespace chip {
namespace DeviceLayer {

/**
 * Called whenever a log message is emitted by Chip or LwIP.
 *
 * This function is intended be overridden by the application to, e.g.,
 * schedule output of queued log entries.
 */
void __attribute__((weak)) OnLogOutput(void) {}

} // namespace DeviceLayer
} // namespace chip

namespace chip {
namespace Logging {
namespace Platform {

/**
 * CHIP log output functions.
 */
void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
#if ASR_LOG_ENABLED && _CHIP_USE_LOGGING
#if ((CFG_EASY_LOG_ENABLE == 1) && (CFG_EASY_LOG_MODULE_EN == 1))
    char log_tag[10];
    snprintf(log_tag, sizeof(log_tag), "CHIP:%s", module);
    switch (category)
    {
    case kLogCategory_Error:
        mlog_e(log_tag, msg, v);
        break;
    case kLogCategory_Progress:
        mlog_i(log_tag, msg, v);
        break;
    case kLogCategory_Detail:
        mlog_v(log_tag, msg, v);
        break;
    case kLogCategory_Automation:
        mlog_d(log_tag, msg, v);
        break;
    case kLogCategory_None:
        mlog_w(log_tag, msg, v);
        break;
    default:
        mlog_a(log_tag, msg, v);
        break;
    }
#else
    printf("CHIP:%s: ", module);
    vprintf(msg, v);
    printf("\n");
#endif
    // Let the application know that a log message has been emitted.
    DeviceLayer::OnLogOutput();
#endif
}

void LogV2(const char * module, const char * msg, va_list v)
{
#if ((CFG_EASY_LOG_ENABLE == 1) && (CFG_EASY_LOG_MODULE_EN == 1))
    elog_chip_printf(module, msg, v);
#else
    printf("%s: ", module);
    vprintf(msg, v);
    printf("\n");
#endif
}

extern "C" void ASR_LOG(const char * aFormat, ...)
{
    va_list v;
    va_start(v, aFormat);
    LogV2("ASR:", aFormat, v);
    va_end(v);
}

} // namespace Platform
} // namespace Logging
} // namespace chip
