/*
 *
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
 *    All rights reserved.
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
 *      platform-independent logging interface for the Weave SDK.
 *
 */

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include <Weave/Support/NLDLLUtil.h>
#include <Weave/Core/WeaveCore.h>
#include <Weave/Support/CodeUtils.h>
#include "WeaveLogging.h"

#if WEAVE_LOGGING_STYLE_ANDROID && defined(__ANDROID__)
#include <android/log.h>
#endif

#if HAVE_SYS_TIME_H && WEAVE_LOGGING_STYLE_STDIO_WITH_TIMESTAMPS
#include <sys/time.h>
#endif // HAVE_SYS_TIME_H && WEAVE_LOGGING_STYLE_STDIO_WITH_TIMESTAMPS

namespace nl {
namespace Weave {
namespace Logging {

#if _WEAVE_USE_LOGGING

/*
 * Array of strings containing the names for each of the Weave log
 * modules.
 *
 * NOTE: The names must be in the order defined in the LogModule
 *       enumeration. Each name must be a fixed number of characters
 *       long (nlWeaveLoggingModuleNameLen) padded with nulls as
 *       necessary.
 *
 */
static const char ModuleNames[] =
    "-\0\0" // None
    "IN\0"  // Inet
    "BLE"   // BLE
    "ML\0"  // MessageLayer
    "SM\0"  // SecurityManager
    "EM\0"  // ExchangeManager
    "TLV"   // TLV
    "ASN"   // ASN1
    "CR\0"  // Crypto
    "DM\0"  // DeviceManager
    "AL\0"  // Alarm
    "BDX"   // BulkDataTransfer
    "DMG"   // DataManagement
    "DC\0"  // DeviceControl
    "DD\0"  // DeviceDescription
    "ECH"   // Echo
    "FP\0"  // FabricProvisioning
    "NP\0"  // NetworkProvisioning
    "SD\0"  // ServiceDirectory
    "SP\0"  // ServiceProvisioning
    "SWU"   // SoftwareUpdate
    "TP\0"  // TokenPairing
    "HL\0"  // HeatLink
    "TS\0"  // TimeServices
    "WT\0"  // WeaveTunnel
    "HB\0"  // Heartbeat
    "WSL"   // WeaveSystemLayer
    "DLP"   // DropcamLegacyPairing
    "EVL"   // Event Logging
    "SPT"   // Support
    ;

#define ModuleNamesCount ((sizeof(ModuleNames) - 1) / nlWeaveLoggingModuleNameLen)

#define WeavePrefix "WEAVE:"
#define WeavePrefixSeparator ": "
#define WeaveMessageTrailer "\n"

void GetModuleName(char *buf, uint8_t module)
{
    const char *moduleNamePtr = ModuleNames + ((module < ModuleNamesCount) ? module * nlWeaveLoggingModuleNameLen : 0);
    memcpy(buf, moduleNamePtr, nlWeaveLoggingModuleNameLen);
    buf[nlWeaveLoggingModuleNameLen] = 0;
}

void GetMessageWithPrefix(char *buf, uint8_t bufSize, uint8_t module, const char *msg)
{
    char moduleName[nlWeaveLoggingModuleNameLen + 1];

    GetModuleName(moduleName, module);
    snprintf(buf, bufSize, WeavePrefix "%s" WeavePrefixSeparator "%s" WeaveMessageTrailer, moduleName, msg);
}

void PrintMessagePrefix(uint8_t module)
{
    char moduleName[nlWeaveLoggingModuleNameLen + 1];
    GetModuleName(moduleName, module);

#if WEAVE_LOGGING_STYLE_STDIO_WITH_TIMESTAMPS
    struct timeval tv;
    struct tm* time_ptr;
    char detailed_time[30];
    int64_t milliseconds;
    int status;

    status = gettimeofday(&tv, NULL);
    VerifyOrExit(status == 0, perror("gettimeofday"));

    time_ptr = localtime(&tv.tv_sec);
    VerifyOrExit(time_ptr != NULL, status = -1; perror("localtime"));

    status = strftime(detailed_time, sizeof(detailed_time), "%F %T%z", time_ptr);
    VerifyOrExit(status >= 0, perror("strftime"));

    milliseconds = tv.tv_usec / 1000;
    printf("%s.%03ld " WeavePrefix "%s: ", detailed_time, milliseconds, moduleName);

exit:
    if (status < 0)
    {
        printf("\?\?\?\?-\?\?-\?\? \?\?:\?\?:\?\?.\?\?\?+\?\?\?\?" WeavePrefix "%s: ", moduleName);
    }

#else // !WEAVE_LOGGING_STYLE_STDIO_WITH_TIMESTAMPS

    printf(WeavePrefix "%s: ", moduleName);

#endif // WEAVE_LOGGING_STYLE_STDIO_WITH_TIMESTAMPS
}

#if WEAVE_LOG_FILTERING

uint8_t gLogFilter = kLogCategory_Max;

#endif // WEAVE_LOG_FILTERING

#if !WEAVE_LOGGING_STYLE_EXTERNAL
/*
 * Only enable an in-package implementation of the logging interface
 * if external logging was not requested. Within that, the package
 * supports either Android-style or C Standard I/O-style logging.
 *
 * In the event a "weak" variant is specified, i.e
 * WEAVE_LOGGING_STYLE_STDIO_WEAK, the in-package implementation will
 * be provided but with "weak" linkage
 */

/**
 * Log, to the platform-specified mechanism, the specified log
 * message, @a msg, for the specified module, @a module, in the
 * provided category, @a category.
 *
 * @param[in] module    A LogModule enumeration indicating the
 *                      source of the Weave package module that
 *                      generated the log message. This must be
 *                      translated within the function to a module
 *                      name for inclusion in the log message.
 * @param[in] category  A LogCategory enumeration indicating the
 *                      category of the log message. The category
 *                      may be filtered in or out if
 *                      WEAVE_LOG_FILTERING was asserted.
 * @param[in] msg       A pointer to a NULL-terminated C string with
 *                      C Standard Library-style format specifiers
 *                      containing the log message to be formatted and
 *                      logged.
 * @param[in] ...       A variadic argument list whose elements should
 *                      correspond to the format specifiers in @a msg.
 *
 */

#if WEAVE_LOGGING_STYLE_STDIO_WEAK
#define __WEAVE_LOGGING_LINK_ATTRIBUTE __attribute__((weak))
#else
#define __WEAVE_LOGGING_LINK_ATTRIBUTE
#endif

NL_DLL_EXPORT __WEAVE_LOGGING_LINK_ATTRIBUTE void Log(uint8_t module, uint8_t category, const char *msg, ...)
{
    va_list v;

    va_start(v, msg);

    if (IsCategoryEnabled(category))
    {

#if WEAVE_LOGGING_STYLE_ANDROID

        char moduleName[nlWeaveLoggingModuleNameLen + 1];
        GetModuleName(moduleName, module);

        int priority = (category == kLogCategory_Error) ? ANDROID_LOG_ERROR : ANDROID_LOG_DEBUG;

        __android_log_vprint(priority, moduleName, msg, v);

#elif WEAVE_LOGGING_STYLE_STDIO || WEAVE_LOGGING_STYLE_STDIO_WEAK

        PrintMessagePrefix(module);
        vprintf(msg, v);
        printf("\n");

#else

#error "Undefined platform-specific implementation for non-externnal Weave logging style!"

#endif /* WEAVE_LOGGING_STYLE_ANDROID */

    }

    va_end(v);
}
#endif /* !WEAVE_LOGGING_STYLE_EXTERNAL */

NL_DLL_EXPORT uint8_t GetLogFilter()
{
#if WEAVE_LOG_FILTERING
    return gLogFilter;
#else
    return kLogCategory_Max;
#endif
}

NL_DLL_EXPORT void SetLogFilter(uint8_t category)
{
#if WEAVE_LOG_FILTERING
    gLogFilter = category;
#endif
}

#endif /* _WEAVE_USE_LOGGING */

} // namespace Logging
} // namespace Weave
} // namespace nl
