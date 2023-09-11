/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <platform/logging/LogV.h>

namespace chip {
namespace Logging {
namespace Platform {

void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    // ChipPlatformLog expands to an os_log call directly (see Logging.h), so
    // we don't need to do anything further here. However his function and the
    // call to it still exist because of scenarios where a different logging
    // backend (usually stdio) is swapped in at link time, e.g. for unit tests.
}

} // namespace Platform
} // namespace Logging
} // namespace chip
