/* See Project CHIP LICENSE file for licensing information. */
#include <platform/logging/LogV.h>

#include <core/CHIPConfig.h>
#include <support/logging/Constants.h>

#include <cstdio>
#include <ctype.h>
#include <string.h>
#include "chip_porting.h"

#ifdef PW_RPC_ENABLED
#include "PigweedLogger.h"
#endif

#include <openthread/logging.h>
#include <openthread/cli.h>

extern "C" int dbg_vprintf(const char * module, const char *format, va_list va);

namespace chip {
namespace DeviceLayer {

/**
 * Called whenever a log message is emitted by chip or LwIP.
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

void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    DBG_TEXT_COMBINE_LEVEL_ERROR(MODULE_MATTER, module, msg, v);
    // uint8_t level = otLoggingGetLevel();
    // if (category && category <= level)
    // {
    //     dbg_vprintf(module, msg, v);  
    // }
}

} // namespace Platform
} // namespace Logging
} // namespace chip
