/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "lib/support/logging/Constants.h"
#include <lib/support/logging/TextOnlyLogging.h>
#include <pw_log/levels.h>

// Pigweed defines more granular logging than CHIP.
// we match the following:
//    - Detail == DEBUG
//    - Progress == INFO
//    - ERROR = WARN, ERROR, CRITICAL

constexpr chip::Logging::LogCategory PwLogLevelToChipLogCategory(int level)
{
    switch (level)
    {
    case PW_LOG_LEVEL_DEBUG:
        return chip::Logging::kLogCategory_Detail;
    case PW_LOG_LEVEL_INFO:
        return chip::Logging::kLogCategory_Progress;
    default:
        return chip::Logging::kLogCategory_Error;
    }
}

#define PW_LOG_DEBUG(...) ChipLogDetail(NotSpecified, __VA_ARGS__)
#define PW_LOG_INFO(...) ChipLogProgress(NotSpecified, __VA_ARGS__)
#define PW_LOG_WARN(...) ChipLogError(NotSpecified, __VA_ARGS__)
#define PW_LOG_ERROR(...) ChipLogError(NotSpecified, __VA_ARGS__)
#define PW_LOG_CRITICAL(...) ChipLogError(NotSpecified, __VA_ARGS__)

// Log a message with many attributes included.
//
// This is the main macro that functions not included above will use
#define PW_HANDLE_LOG(level, module, flags, message, ...)                                                                          \
    ChipInternalLogImpl(NotSpecified, PwLogLevelToChipLogCategory(level), message, __VA_ARGS__)
