/* See Project CHIP LICENSE file for licensing information. */

#include <platform/logging/LogV.h>

#include <lib/core/CHIPConfig.h>
#include <lib/support/EnforceFormat.h>
#include <lib/support/logging/Constants.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/cbprintf.h>

// If CONFIG_LOG_MODE_MINIMAL the timestamp is NOT added automatically by the Zephyr logger
#ifdef CONFIG_LOG_MODE_MINIMAL
#define LOG_FORMAT "%u %s"
#define LOG_MESSAGE(msg) k_uptime_get_32(), (msg)
#else
#define LOG_FORMAT "%s"
#define LOG_MESSAGE(msg) (msg)
#endif

LOG_MODULE_REGISTER(chip, CONFIG_MATTER_LOG_LEVEL);

namespace chip {
namespace DeviceLayer {

/**
 * Called whenever a log message is emitted by chip.
 *
 * This function is intended be overridden by the application to, e.g.,
 * schedule output of queued log entries.
 */
void __attribute__((weak)) OnLogOutput(void) {}

} // namespace DeviceLayer

namespace Logging {
namespace Platform {

/**
 * CHIP log output function.
 */

void ENFORCE_FORMAT(3, 0) LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
    snprintfcb(formattedMsg, sizeof(formattedMsg), "[%s]", module);

    const size_t prefixLen = strlen(formattedMsg);
    vsnprintfcb(formattedMsg + prefixLen, sizeof(formattedMsg) - prefixLen, msg, v);

    const char * allocatedMsg = formattedMsg;

    // Invoke the Zephyr logging library to log the message.
    //
    // Unfortunately the Zephyr logging macros end up assigning uint16_t
    // variables to uint16_t:10 fields, which triggers integer conversion
    // warnings.  And treating the Zephyr headers as system headers does not
    // help, apparently.  Just turn off that warning around this switch.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    switch (category)
    {
    case kLogCategory_Error:
        LOG_ERR(LOG_FORMAT, LOG_MESSAGE(allocatedMsg));
        break;
    case kLogCategory_Progress:
    default:
        LOG_INF(LOG_FORMAT, LOG_MESSAGE(allocatedMsg));
        break;
    case kLogCategory_Detail:
        LOG_DBG(LOG_FORMAT, LOG_MESSAGE(allocatedMsg));
        break;
    }
#pragma GCC diagnostic pop

    // Let the application know that a log message has been emitted.
    DeviceLayer::OnLogOutput();
}

} // namespace Platform
} // namespace Logging
} // namespace chip
