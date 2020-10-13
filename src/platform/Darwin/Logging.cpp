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
 *          Provides implementations for the CHIP logging functions on
 *          Darwin platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <support/logging/CHIPLogging.h>

#include <os/log.h>

/**
 * CHIP log output functions.
 */
namespace chip {
namespace Logging {
void LogV(uint8_t module, uint8_t category, const char * msg, va_list v)
{
    if (IsCategoryEnabled(category))
    {
        char moduleName[ChipLoggingModuleNameLen + 1];
        GetModuleName(moduleName, module);

        char formattedMsg[512];
        int32_t prefixLen = snprintf(formattedMsg, sizeof(formattedMsg), "CHIP: [%s] ", moduleName);
        if (prefixLen < 0)
        {
            // This should never happens.
            return;
        }

        if (prefixLen >= sizeof(formattedMsg))
        {
            prefixLen = sizeof(formattedMsg) - 1;
        }

        vsnprintf(formattedMsg + prefixLen, sizeof(formattedMsg) - static_cast<size_t>(prefixLen), msg, v);

        switch (category)
        {
        case kLogCategory_Error:
            os_log_with_type(OS_LOG_DEFAULT, OS_LOG_TYPE_ERROR, "🔴 %{public}s", formattedMsg);
#if TARGET_OS_MAC && TARGET_OS_IPHONE == 0
            fprintf(stdout, "\033[1;31m");
#endif
            break;

        case kLogCategory_Progress:
            os_log_with_type(OS_LOG_DEFAULT, OS_LOG_TYPE_INFO, "🔵 %{public}s", formattedMsg);
#if TARGET_OS_MAC && TARGET_OS_IPHONE == 0
            fprintf(stdout, "\033[0;34m");
#endif
            break;

        case kLogCategory_Detail:
            os_log_with_type(OS_LOG_DEFAULT, OS_LOG_TYPE_DEBUG, "🟢 %{public}s", formattedMsg);
#if TARGET_OS_MAC && TARGET_OS_IPHONE == 0
            fprintf(stdout, "\033[0;32m");
#endif
            break;
        }
#if TARGET_OS_MAC && TARGET_OS_IPHONE == 0
        fprintf(stdout, "%s\n\033[0m", formattedMsg);
#endif
    }
}
} // namespace Logging
} // namespace chip
