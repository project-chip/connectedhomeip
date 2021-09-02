#include <lib/support/logging/CHIPLogging.h>
#include <platform/logging/LogV.h>
#include <stdio.h>

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
#if P6_LOG_ENABLED && _CHIP_USE_LOGGING
    printf("CHIP:%s: ", module);
    vprintf(msg, v);
    printf("\n");
    // Let the application know that a log message has been emitted.
    DeviceLayer::OnLogOutput();
#endif
}

extern "C" void P6Log(const char * aFormat, ...)
{
    va_list v;
    va_start(v, aFormat);
    LogV("P6", chip::Logging::kLogCategory_Progress, aFormat, v);
    va_end(v);
}

} // namespace Platform
} // namespace Logging
} // namespace chip
