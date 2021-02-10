/* See Project CHIP LICENSE file for licensing information. */


/**
 *    @file
 *          Provides implementations for the CHIP logging functions
 *          on Zephyr platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <support/logging/CHIPLogging.h>

#include <kernel.h>
#include <logging/log.h>

#include <cstdio>

using namespace ::chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;

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

/**
 * CHIP log output function.
 */

void LogV(uint8_t module, uint8_t category, const char * msg, va_list v)
{
    if (!IsCategoryEnabled(category))
        return;

    {
        char formattedMsg[CHIP_DEVICE_CONFIG_LOG_MESSAGE_MAX_SIZE];
        size_t prefixLen = 0;

        // Max size for "[TAG] {UINT32}"
        constexpr size_t maxPrefixLen = chip::Logging::kMaxModuleNameLen + 10 + 3;
        static_assert(sizeof(formattedMsg) > maxPrefixLen);

        prefixLen += snprintf(formattedMsg, sizeof(formattedMsg), "%u", k_uptime_get_32());

        // Form the log prefix, e.g. "[DL] "
        formattedMsg[prefixLen++] = '[';
        GetModuleName(formattedMsg + prefixLen, chip::Logging::kMaxModuleNameLen + 1, module);
        prefixLen                 = strlen(formattedMsg);
        formattedMsg[prefixLen++] = ']';
        formattedMsg[prefixLen++] = ' ';

        // Append the log message.
        vsnprintf(formattedMsg + prefixLen, sizeof(formattedMsg) - prefixLen, msg, v);

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
    }

    // Let the application know that a log message has been emitted.
    DeviceLayer::OnLogOutput();
}

} // namespace Logging
} // namespace chip
