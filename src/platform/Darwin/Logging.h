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

#pragma once

#include <lib/support/logging/Constants.h>

#include <os/log.h>

#ifdef __OBJC__
@class NSString;
#endif // __OBJC__

#define ChipPlatformLog(MOD, CAT, MSG, ...)                                                                                        \
    do                                                                                                                             \
    {                                                                                                                              \
        ChipPlatformValidateLogFormat(MSG, ##__VA_ARGS__); /* validate once and ignore warnings from os_log() / Log() */           \
        _Pragma("clang diagnostic push");                                                                                          \
        _Pragma("clang diagnostic ignored \"-Wformat\"");                                                                          \
        os_log_with_type(chip::Logging::Platform::LoggerForModule(chip::Logging::kLogModule_##MOD, #MOD),                          \
                         static_cast<os_log_type_t>(chip::Logging::Platform::kOSLogCategory_##CAT), MSG, ##__VA_ARGS__);           \
        ChipInternalLogImpl(MOD, CHIP_LOG_CATEGORY_##CAT, MSG, ##__VA_ARGS__);                                                     \
        _Pragma("clang diagnostic pop");                                                                                           \
    } while (0)

#define ChipPlatformLogByteSpan(MOD, CAT, DATA)                                                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        chip::Logging::Platform::LogByteSpan(chip::Logging::kLogModule_##MOD, #MOD,                                                \
                                             static_cast<os_log_type_t>(chip::Logging::Platform::kOSLogCategory_##CAT), DATA);     \
        ChipInternalLogByteSpanImpl(MOD, CHIP_LOG_CATEGORY_##CAT, DATA);                                                           \
    } while (0)

namespace chip {

// Avoid includes cycle: Span.h indirectly includes CHIPLogging.h
template <class T>
class Span;
using ByteSpan = Span<const uint8_t>;

namespace Logging {
namespace Platform {

// Names align with chip::Logging::LogCategory
enum OSLogCategory
{
    kOSLogCategory_ERROR      = OS_LOG_TYPE_ERROR,
    kOSLogCategory_PROGRESS   = OS_LOG_TYPE_DEFAULT,
    kOSLogCategory_DETAIL     = OS_LOG_TYPE_INFO,
    kOSLogCategory_AUTOMATION = OS_LOG_TYPE_DEFAULT,
};

os_log_t LoggerForModule(chip::Logging::LogModule moduleId, char const * moduleName);
void LogByteSpan(chip::Logging::LogModule moduleId, char const * moduleName, os_log_type_t type, const chip::ByteSpan & span);

// Helper constructs for compile-time validation of format strings for C++ / ObjC++ contexts.
// Note that ObjC++ contexts are restricted to NSString style specifiers. Supporting os_log()
// specifiers would require these to be emulated or stripped when log redirection is used.
#ifdef __OBJC__
bool ValidateLogFormat(NSString * format, ...) __attribute__((format(__NSString__, 1, 0))); // not implemented
#define ChipPlatformValidateLogFormat(F, ...) ((void) sizeof(chip::Logging::Platform::ValidateLogFormat(@F, ##__VA_ARGS__)))
#else // __OBJC__
bool ValidateLogFormat(char const * format, ...) __attribute__((format(printf, 1, 0))); // not implemented
#define ChipPlatformValidateLogFormat(F, ...) ((void) sizeof(chip::Logging::Platform::ValidateLogFormat(F, ##__VA_ARGS__)))
#endif // __OBJC__

} // namespace Platform
} // namespace Logging
} // namespace chip
