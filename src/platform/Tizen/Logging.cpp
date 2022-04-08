/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <platform/logging/LogV.h>

#include <lib/core/CHIPConfig.h>
#include <lib/support/EnforceFormat.h>
#include <lib/support/logging/Constants.h>

#include <dlog.h>
#include <stdio.h>

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
