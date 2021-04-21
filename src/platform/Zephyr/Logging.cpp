/* See Project CHIP LICENSE file for licensing information. */

#include <platform/logging/LogV.h>

#include <core/CHIPConfig.h>
#include <support/logging/Constants.h>

#include <kernel.h>
#include <logging/log.h>
#include <sys/cbprintf.h>

LOG_MODULE_REGISTER(chip, LOG_LEVEL_DBG);

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

void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];

    snprintfcb(formattedMsg, sizeof(formattedMsg), "%u [%s]", k_uptime_get_32(), module);

    // -2 to ensure at least one byte available for vsnprintf below.
    formattedMsg[sizeof(formattedMsg) - 2] = 0;

    size_t prefixLen = strlen(formattedMsg);

    // Append the log message.
    vsnprintfcb(formattedMsg + prefixLen, sizeof(formattedMsg) - prefixLen, msg, v);

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
        LOG_ERR("%s", log_strdup(formattedMsg));
        break;
    case kLogCategory_Progress:
    default:
        LOG_INF("%s", log_strdup(formattedMsg));
        break;
    case kLogCategory_Detail:
        LOG_DBG("%s", log_strdup(formattedMsg));
        break;
    }
#pragma GCC diagnostic pop

    // Let the application know that a log message has been emitted.
    DeviceLayer::OnLogOutput();
}

} // namespace Platform
} // namespace Logging
} // namespace chip
