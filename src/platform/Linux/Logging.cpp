/* See Project CHIP LICENSE file for licensing information. */


/**
 *    @file
 *          Provides implementations for the CHIP and LwIP logging functions
 *          on Linux platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <support/logging/CHIPLogging.h>

#include <assert.h>
#include <stdarg.h>
#include <syslog.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;

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
} // namespace chip

namespace chip {
namespace Logging {

/**
 * CHIP log output functions.
 */
void LogV(uint8_t module, uint8_t category, const char * msg, va_list v)
{
    if (IsCategoryEnabled(category))
    {
        vsyslog(LOG_INFO, msg, v);

        // Let the application know that a log message has been emitted.
        DeviceLayer::OnLogOutput();
    }
}

} // namespace Logging
} // namespace chip
