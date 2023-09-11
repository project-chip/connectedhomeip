/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cstdint>
#include <cstdio>

#include <dlog.h>

#include <core/CHIPBuildConfig.h>
#include <lib/support/EnforceFormat.h>
#include <lib/support/logging/Constants.h>
#include <platform/logging/LogV.h>

namespace chip {
namespace Logging {
namespace Platform {

/**
 * CHIP log output functions.
 */
void ENFORCE_FORMAT(3, 0) LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    constexpr const char * kLogTag                = "CHIP";
    char msgBuf[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE] = {
        0,
    };
    vsnprintf(msgBuf, sizeof(msgBuf), msg, v);

    switch (category)
    {
    case kLogCategory_Error:
        dlog_print(DLOG_ERROR, kLogTag, "%s: %s", module, msgBuf);
        break;
    case kLogCategory_Detail:
        dlog_print(DLOG_DEBUG, kLogTag, "%s: %s", module, msgBuf);
        break;
    case kLogCategory_Progress:
    default:
        dlog_print(DLOG_INFO, kLogTag, "%s: %s", module, msgBuf);
        break;
    }
}

} // namespace Platform
} // namespace Logging
} // namespace chip
