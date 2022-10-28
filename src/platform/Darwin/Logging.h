/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *          Platform-specific configuration overrides for the CHIP BLE
 *          Layer on Darwin platforms.
 *
 */

#pragma once

#include <lib/support/EnforceFormat.h>
#include <stdarg.h>
#include <stdint.h>

namespace chip {
namespace Logging {
namespace Platform {

/**
 * Get the standardized log message format appropriate for the Darwin platform.
 *
 * @param[in] module        The name of the log module.
 * @param[in] category      A LogCategory enumeration indicating the
 *                          category of the log message. The category
 *                          may be filtered in or out if
 *                          CHIP_LOG_FILTERING was asserted.
 * @param[in] msg           A pointer to a NULL-terminated C string with
 *                          C Standard Library-style format specifiers
 *                          containing the log message to be formatted and
 *                          logged.
 * @param[in] v             A variadic argument list whose elements should
 *                          correspond to the format specifiers in @a msg.
 * @param[in] size          The size of the formatted msg string.
 * @param[out] formattedMsg The address of the log message to write the log message format to.
 *
 */
void ENFORCE_FORMAT(3, 0) getDarwinLogMessageFormat(const char * module, uint8_t category, const char * msg, va_list v,
                                                    uint16_t size, char * formattedMsg);

} // namespace Platform
} // namespace Logging
} // namespace chip
