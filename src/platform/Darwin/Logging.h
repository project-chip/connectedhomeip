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

// CHIP_SYSTEM_CONFIG_PLATFORM_LOG API ////////////////////////////////////////////////////////////////////////////////

#define ChipPlatformLog(MOD, CAT, MSG, ...)                                                                                        \
    do                                                                                                                             \
    {                                                                                                                              \
        ChipPlatformValidateLogFormat(MSG, ##__VA_ARGS__); /* validate once and ignore warnings from os_log() / CHIP Log() */      \
        _Pragma("clang diagnostic push");                                                                                          \
        _Pragma("clang diagnostic ignored \"-Wformat\"");                                                                          \
        ChipPlatformLogImpl(ChipPlatformLogger(MOD), ChipPlatformLogCategory(CAT), MSG, ##__VA_ARGS__); /* os_log() */             \
        OS_LOG_STRING(LOG, _chip_fmt_str, MSG); /* avoid separate duplicate string constants (allow de-dupe) */                    \
        ChipInternalLogImpl(MOD, CHIP_LOG_CATEGORY_##CAT, _chip_fmt_str, ##__VA_ARGS__); /* CHIP Log() */                          \
        _Pragma("clang diagnostic pop");                                                                                           \
    } while (0)

#define ChipPlatformLogByteSpan(MOD, CAT, DATA)                                                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        chip::Logging::Platform::LogByteSpan(chip::Logging::kLogModule_##MOD, ChipPlatformLogCategory(CAT), DATA);                 \
        ChipInternalLogByteSpanImpl(MOD, CHIP_LOG_CATEGORY_##CAT, DATA);                                                           \
    } while (0)

// Implementation /////////////////////////////////////////////////////////////////////////////////////////////////////

namespace chip {

// Avoid includes cycle: Span.h indirectly includes CHIPLogging.h
template <class T>
class Span;
using ByteSpan = Span<const uint8_t>;

namespace Logging {
namespace Platform {

// Map chip::Logging::LogCategory suffixes to os_log_type_t constants
#define ChipPlatformLogCategory(CAT) chip::Logging::Platform::kOSLogCategory_##CAT
inline constexpr auto kOSLogCategory_ERROR      = OS_LOG_TYPE_ERROR;
inline constexpr auto kOSLogCategory_PROGRESS   = OS_LOG_TYPE_DEFAULT;
inline constexpr auto kOSLogCategory_DETAIL     = OS_LOG_TYPE_INFO;
inline constexpr auto kOSLogCategory_AUTOMATION = OS_LOG_TYPE_DEFAULT;

// Some contortions are necessary to avoid unnecessary retain / release calls on os_log_t
// in ObjC++ callers. __unsafe_unretained cannot be used in return types, so LoggerForModule()
// returns a void pointer, which we then cast back to os_log_t.
#define ChipPlatformLogger(MOD) chip::Logging::Platform::LoggerForModule(chip::Logging::kLogModule_##MOD)
__attribute__((const)) void * LoggerForModule(LogModule moduleId);

#if !defined(__OBJC__) || !__has_feature(objc_arc)
// These are just wrappers around os_log_enabled / os_log_with_type with a cast to os_log_t
#define ChipPlatformLogEnabled(log, type) os_log_type_enabled((os_log_t) log, type)
#define ChipPlatformLogImpl(log, type, fmt, ...) os_log_with_type((os_log_t) log, type, fmt, ##__VA_ARGS__)
#else
// For ObjC++ with ARC, clang will call _objc_retainAutoreleasedReturnValue() on the return value
// of LoggerForModule() when it is assigned to the strong local os_log_t variable created by the
// os_log_with_type() macro. To avoid this, we re-define os_log_t within the block as unretained;
// this is a bit messy, but the only other option appears to be to define a custom copy of
// os_log_with_type(), which is also not ideal.
#define ChipPlatformLogEnabled(log, type) os_log_type_enabled((__bridge os_log_t) log, type)
#define ChipPlatformLogImpl(log, type, fmt, ...)                                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        _Pragma("clang diagnostic push");                                                                                          \
        _Pragma("clang diagnostic ignored \"-Wshadow\"") /* issue #39135 */ using os_log_t = ::os_log_t __unsafe_unretained;       \
        os_log_with_type((__bridge os_log_t) log, type, fmt, ##__VA_ARGS__);                                                       \
        _Pragma("clang diagnostic pop");                                                                                           \
    } while (0)
#endif

void LogByteSpan(LogModule moduleId, os_log_type_t type, const chip::ByteSpan & span);

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
