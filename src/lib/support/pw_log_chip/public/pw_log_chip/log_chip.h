/*
 *    Copyright (c) 2020 Project CHIP Authors.
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
 *        This header redirects Pigweed logging prints to chip.
 */
#pragma once

#include <lib/support/logging/CHIPLogging.h>

#include "pw_preprocessor/arguments.h"

/**
 * Redirect pigweed log prints to chip logging.
 * Currently does not use a log module, but this could be added when needed.
 * pigweed debug and info level logs are mapped to chip's kLogCategory_Detail.
 * All other log levels are mapped to chip's kLogCategory_Error.
 *
 * Note: This function should not be called directly, instead call the functions
 *       in pw_log/log.h.
 *
 * @param[in] level    Pigweed log level.
 * @param[in] flags    Pigweed logging flags, currently these are ignored.
 * @param[in] message  The printf style log string.
 */
#define PW_LOG(level, flags, message, ...)                                                                                         \
    do                                                                                                                             \
    {                                                                                                                              \
        if (level >= PW_LOG_LEVEL_INFO)                                                                                            \
        {                                                                                                                          \
            ::chip::Logging::Log(::chip::Logging::kLogModule_NotSpecified,                                                         \
                                 (level <= PW_LOG_LEVEL_INFO) ? ::chip::Logging::kLogCategory_Detail                               \
                                                              : ::chip::Logging::kLogCategory_Error,                               \
                                 message PW_COMMA_ARGS(__VA_ARGS__));                                                              \
        }                                                                                                                          \
    } while (0)
