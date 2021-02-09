/* See Project chip LICENSE file for licensing information. */

#include "CHIPLogging.h"

#include <core/CHIPCore.h>
#include <os/log.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

namespace chip {
namespace Logging {

void LogV(uint8_t module, uint8_t category, const char * msg, va_list v)
{
    if (!IsCategoryEnabled(category))
    {
        return;
    }

    char moduleName[chip::Logging::kMaxModuleNameLen + 1];
    GetModuleName(moduleName, sizeof(moduleName), module);

    char formattedMsg[512];
    int32_t prefixLen = snprintf(formattedMsg, sizeof(formattedMsg), "CHIP: [%s] ", moduleName);
    if (prefixLen < 0)
    {
        // This should not happen
        return;
    }

    if (static_cast<size_t>(prefixLen) >= sizeof(formattedMsg))
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
        fprintf(stdout, "\033[0;32m");
#endif
        break;

    case kLogCategory_Detail:
        os_log_with_type(OS_LOG_DEFAULT, OS_LOG_TYPE_DEBUG, "🟢 %{public}s", formattedMsg);
#if TARGET_OS_MAC && TARGET_OS_IPHONE == 0
        fprintf(stdout, "\033[0;34m");
#endif
        break;
    }
#if TARGET_OS_MAC && TARGET_OS_IPHONE == 0
    fprintf(stdout, "%s\033[0m\n", formattedMsg);
#endif
}

} // namespace Logging
} // namespace chip
