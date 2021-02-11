/* See Project CHIP LICENSE file for licensing information. */

#include <platform/logging/LogV.h>

#include <stdio.h>

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
    printf("CHIP:%s: ", module);
    vprintf(msg, v);
    printf("\n");

    // Let the application know that a log message has been emitted.
    DeviceLayer::OnLogOutput();
}

} // namespace Platform
} // namespace Logging
} // namespace chip
