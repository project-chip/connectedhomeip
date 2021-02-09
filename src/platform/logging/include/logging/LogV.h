/* See Project CHIP LICENSE file for licensing information. */

#pragma once

#include <stdarg.h>
#include <stdint.h>

namespace chip {
namespace Logging {
namespace Platform {

/**
 * Log, to the platform-specified mechanism, the specified log
 * message, @a msg, for the specified module, @a module, in the
 * provided category, @a category.
 *
 * @param[in] module    The name of the log module.
 * @param[in] category  A LogCategory enumeration indicating the
 *                      category of the log message. The category
 *                      may be filtered in or out if
 *                      CHIP_LOG_FILTERING was asserted.
 * @param[in] msg       A pointer to a NULL-terminated C string with
 *                      C Standard Library-style format specifiers
 *                      containing the log message to be formatted and
 *                      logged.
 * @param[in] args       A variadic argument list whose elements should
 *                      correspond to the format specifiers in @a msg.
 *
 */
void LogV(const char * module, uint8_t category, const char * msg, va_list args);

} // namespace Platform
} // namespace Logging
} // namespace chip
