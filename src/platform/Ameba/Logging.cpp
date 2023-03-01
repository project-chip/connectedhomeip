/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          Provides implementations for the CHIP logging functions
 *          on the Ameba platform.
 */
/* this file behaves like a config.h, comes first */
#include <platform/logging/LogV.h>

#include <core/CHIPConfig.h>
#include <support/logging/Constants.h>

#include "Logging.h"
#include <stdio.h>

namespace chip {
namespace Logging {
namespace Ameba {

static AmebaLogLevel LogLevel = AmebaLogLevel::kDetail;

void LogSetLevel(uint8_t level)
{
    LogLevel = level;
}

uint8_t LogGetLevel()
{
    return LogLevel;
}

void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    char tag[11];

    snprintf(tag, sizeof(tag), "chip[%s]", module);
    tag[sizeof(tag) - 1] = 0;

    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
    vsnprintf(formattedMsg, sizeof(formattedMsg), msg, v);

    switch (category)
    {
    case kLogCategory_Error:
        if (LogLevel >= AmebaLogLevel::kError)
            printf("%s %s\r\n", tag, formattedMsg);
        break;
    case kLogCategory_Progress:
    default:
        if (LogLevel >= AmebaLogLevel::kProgress)
            printf("%s %s\r\n", tag, formattedMsg);
        break;
    case kLogCategory_Detail:
        if (LogLevel >= AmebaLogLevel::kDetail)
            printf("%s %s\r\n", tag, formattedMsg);
        break;
    }
}

} // namespace Ameba
} // namespace Logging
} // namespace chip
