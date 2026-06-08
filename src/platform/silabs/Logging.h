/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#pragma once

#include <cstddef>
#include <cstdint>

#include <algorithm>

namespace chip {
namespace Logging {
namespace Platform {

// Log category string constants
constexpr const uint8_t kLogError[]  = "[error]";
constexpr const uint8_t kLogWarn[]   = "[warn]";
constexpr const uint8_t kLogInfo[]   = "[info]";
constexpr const uint8_t kLogDetail[] = "[detail]";
constexpr const uint8_t kLogLwip[]   = "[lwip]";
constexpr const uint8_t kLogSilabs[] = "[silabs]";
constexpr const uint8_t kLogNone[]   = "-";
constexpr const uint8_t kOTModule[]  = "[ot]";

static constexpr size_t kTimeStampStringSize = sizeof("[0000:00:00.000]"); // includes null terminator
static constexpr size_t kMaxCategoryStrLen =
    std::max({ sizeof(kLogError), sizeof(kLogWarn), sizeof(kLogInfo), sizeof(kLogDetail), sizeof(kLogLwip), sizeof(kLogSilabs),
               sizeof(kLogNone), sizeof(kOTModule) });

enum LogCategory : uint8_t
{
    kLog_Error    = 0,
    kLog_Progress = 1,
    kLog_Detail   = 2,
    kLog_Warning  = 3,
    kLog_Lwip     = 4,
    kLog_Silabs   = 5,
    kLog_None     = 6,
    kLog_OT       = 7,
};

inline const uint8_t * GetCategoryString(LogCategory category)
{
    switch (category)
    {
    case kLog_Error:
        return (kLogError);
    case kLog_Progress:
        return (kLogInfo);
    case kLog_Detail:
        return (kLogDetail);
    case kLog_Warning:
        return (kLogWarn);
    case kLog_Lwip:
        return (kLogLwip);
    case kLog_Silabs:
        return (kLogSilabs);
    case kLog_OT:
        return (kOTModule);
    default:
        return (kLogNone);
    }
}

inline uint8_t GetCategoryStringSize(LogCategory category)
{
    switch (category)
    {
    case kLog_Error:
        return sizeof(kLogError) - 1;
    case kLog_Progress:
        return sizeof(kLogInfo) - 1;
    case kLog_Detail:
        return sizeof(kLogDetail) - 1;
    case kLog_Lwip:
        return sizeof(kLogLwip) - 1;
    case kLog_Silabs:
        return sizeof(kLogSilabs) - 1;
    case kLog_OT:
        return sizeof(kOTModule) - 1;
    default:
        return sizeof(kLogNone) - 1;
    }
}

/**
 * @brief Add a timestamp in hh:mm:ss.ms format and the given prefix string to the given char buffer
 * The time stamp is derived from the boot time
 *
 * @param logBuffer: pointer to the buffer where to add the information
 * @param prefix: A prefix to add to the trace e.g. The category
 * @param maxSize: Space available in the given buffer.
 * @return size_t: Number of characters written to the buffer
 */
size_t AddTimeStampAndPrefixStr(char * logBuffer, const char * prefix, size_t maxSize);
size_t FormatTimestamp(char * buffer, size_t maxSize, uint64_t timestampMillis);

} // namespace Platform
} // namespace Logging
} // namespace chip
