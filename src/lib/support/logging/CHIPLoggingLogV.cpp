/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file implements macros, constants, and interfaces for a
 *      platform-independent logging interface for the chip SDK.
 *
 */

#include "CHIPLogging.h"

#include <core/CHIPCore.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>

#if CHIP_LOGGING_STYLE_ANDROID && defined(__ANDROID__)
#include <android/log.h>
#endif

#if CHIP_LOGGING_STYLE_DARWIN
#include <os/log.h>
#endif

#if HAVE_SYS_TIME_H && CHIP_LOGGING_STYLE_STDIO_WITH_TIMESTAMPS
#include <sys/time.h>
#endif // HAVE_SYS_TIME_H && CHIP_LOGGING_STYLE_STDIO_WITH_TIMESTAMPS

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

namespace chip {
namespace Logging {

#if _CHIP_USE_LOGGING

#if !CHIP_LOGGING_STYLE_EXTERNAL
/*
 * Only enable an in-package implementation of the logging interface
 * if external logging was not requested. Within that, the package
 * supports either Android-style or C Standard I/O-style logging.
 *
 * In the event a "weak" variant is specified, i.e
 * CHIP_LOGGING_STYLE_STDIO_WEAK, the in-package implementation will
 * be provided but with "weak" linkage
 */

/**
 * Log, to the platform-specified mechanism, the specified log
 * message, @a msg, for the specified module, @a module, in the
 * provided category, @a category.
 *
 * @param[in] module    A LogModule enumeration indicating the
 *                      source of the chip package module that
 *                      generated the log message. This must be
 *                      translated within the function to a module
 *                      name for inclusion in the log message.
 * @param[in] category  A LogCategory enumeration indicating the
 *                      category of the log message. The category
 *                      may be filtered in or out if
 *                      CHIP_LOG_FILTERING was asserted.
 * @param[in] msg       A pointer to a NULL-terminated C string with
 *                      C Standard Library-style format specifiers
 *                      containing the log message to be formatted and
 *                      logged.
 * @param[in] v         A variadic argument list whose elements should
 *                      correspond to the format specifiers in @a msg.
 *
 */

#if CHIP_LOGGING_STYLE_STDIO_WEAK
#define __CHIP_LOGGING_LINK_ATTRIBUTE __attribute__((weak))
#else
#define __CHIP_LOGGING_LINK_ATTRIBUTE
#endif

DLL_EXPORT __CHIP_LOGGING_LINK_ATTRIBUTE void LogV(uint8_t module, uint8_t category, const char * msg, va_list v)
{
    if (IsCategoryEnabled(category))
    {

#if CHIP_LOGGING_STYLE_ANDROID

        char moduleName[ChipLoggingModuleNameLen + 1];
        GetModuleName(moduleName, module);

        int priority = (category == kLogCategory_Error) ? ANDROID_LOG_ERROR : ANDROID_LOG_DEBUG;

        __android_log_vprint(priority, moduleName, msg, v);

#elif CHIP_LOGGING_STYLE_STDIO || CHIP_LOGGING_STYLE_STDIO_WEAK

        PrintMessagePrefix(module);
        vprintf(msg, v);
        printf("\n");

#elif CHIP_LOGGING_STYLE_DARWIN

        char moduleName[ChipLoggingModuleNameLen + 1];
        GetModuleName(moduleName, module);

        char formattedMsg[512];
        int32_t prefixLen = snprintf(formattedMsg, sizeof(formattedMsg), "CHIP: [%s] ", moduleName);
        if (prefixLen < 0)
        {
            // This should never happens.
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

#else

#error "Undefined platform-specific implementation for non-externnal chip logging style!"

#endif /* CHIP_LOGGING_STYLE_ANDROID */
    }
}

#endif /* !CHIP_LOGGING_STYLE_EXTERNAL */

#endif /* _CHIP_USE_LOGGING */

} // namespace Logging
} // namespace chip
