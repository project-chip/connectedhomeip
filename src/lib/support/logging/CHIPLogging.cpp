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

#if HAVE_SYS_TIME_H && CHIP_LOGGING_STYLE_STDIO_WITH_TIMESTAMPS
#include <sys/time.h>
#endif // HAVE_SYS_TIME_H && CHIP_LOGGING_STYLE_STDIO_WITH_TIMESTAMPS

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

namespace chip {
namespace Logging {

#if _CHIP_USE_LOGGING

/*
 * Array of strings containing the names for each of the chip log
 * modules.
 *
 * NOTE: The names must be in the order defined in the LogModule
 *       enumeration. Each name must be a fixed number of characters
 *       long (ChipLoggingModuleNameLen) padded with nulls as
 *       necessary.
 *
 */
static const char ModuleNames[] = "-\0\0" // None
                                  "IN\0"  // Inet
                                  "BLE"   // BLE
                                  "ML\0"  // MessageLayer
                                  "SM\0"  // SecurityManager
                                  "EM\0"  // ExchangeManager
                                  "TLV"   // TLV
                                  "ASN"   // ASN1
                                  "CR\0"  // Crypto
                                  "CTL"   // Controller
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
                                  "TS\0"  // TimeServices
                                  "HB\0"  // Heartbeat
                                  "CSL"   // chipSystemLayer
                                  "EVL"   // Event Logging
                                  "SPT"   // Support
                                  "TOO"   // chipTool
                                  "ZCL"   // Zcl
                                  "SH\0"  // Shell
                                  "DL\0"  // DeviceLayer
                                  "SPL"   // SetupPayload
                                  "SVR"   // AppServer
                                  "DIS"   // Discovery
    ;

#define ModuleNamesCount ((sizeof(ModuleNames) - 1) / ChipLoggingModuleNameLen)

#define chipPrefix "CHIP:"
#define chipPrefixSeparator ": "
#define chipMessageTrailer "\n"

void GetModuleName(char * buf, uint8_t module)
{
    const char * moduleNamePtr = ModuleNames + ((module < ModuleNamesCount) ? module * ChipLoggingModuleNameLen : 0);
    memcpy(buf, moduleNamePtr, ChipLoggingModuleNameLen);
    buf[ChipLoggingModuleNameLen] = 0;
}

void GetMessageWithPrefix(char * buf, uint8_t bufSize, uint8_t module, const char * msg)
{
    char moduleName[ChipLoggingModuleNameLen + 1];

    GetModuleName(moduleName, module);
    snprintf(buf, bufSize, chipPrefix "%s" chipPrefixSeparator "%s" chipMessageTrailer, moduleName, msg);
}

void PrintMessagePrefix(uint8_t module)
{
    char moduleName[ChipLoggingModuleNameLen + 1];
    GetModuleName(moduleName, module);

#if CHIP_LOGGING_STYLE_STDIO_WITH_TIMESTAMPS
    struct timeval tv;
    struct tm * time_ptr;
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
    printf("%s.%03ld " chipPrefix "%s: ", detailed_time, milliseconds, moduleName);

exit:
    if (status < 0)
    {
        printf("\?\?\?\?-\?\?-\?\? \?\?:\?\?:\?\?.\?\?\?+\?\?\?\?" chipPrefix "%s: ", moduleName);
    }

#else // !CHIP_LOGGING_STYLE_STDIO_WITH_TIMESTAMPS

    printf(chipPrefix "%s: ", moduleName);

#endif // CHIP_LOGGING_STYLE_STDIO_WITH_TIMESTAMPS
}

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
 * @param[in] ...       A variadic argument list whose elements should
 *                      correspond to the format specifiers in @a msg.
 *
 */
DLL_EXPORT void Log(uint8_t module, uint8_t category, const char * msg, ...)
{
    va_list v;

    va_start(v, msg);

    LogV(module, category, msg, v);

    va_end(v);
}

#if CHIP_LOG_FILTERING
uint8_t gLogFilter = kLogCategory_Max;
DLL_EXPORT bool IsCategoryEnabled(uint8_t category)
{
    return (category <= gLogFilter);
}

DLL_EXPORT uint8_t GetLogFilter()
{
    return gLogFilter;
}

DLL_EXPORT void SetLogFilter(uint8_t category)
{
    gLogFilter = category;
}

#else  // CHIP_LOG_FILTERING

DLL_EXPORT bool IsCategoryEnabled(uint8_t category)
{
    (void) category;
    return true;
}

DLL_EXPORT uint8_t GetLogFilter()
{
    return kLogCategory_Max;
}

DLL_EXPORT void SetLogFilter(uint8_t category)
{
    (void) category;
}
#endif // CHIP_LOG_FILTERING

#endif /* _CHIP_USE_LOGGING */

} // namespace Logging
} // namespace chip
