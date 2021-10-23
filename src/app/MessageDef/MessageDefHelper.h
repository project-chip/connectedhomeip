/**
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC.
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      This file defines message helper functions in CHIP interaction model
 *
 */

#pragma once

#include <algorithm>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppBuildConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK && CHIP_DETAIL_LOGGING
void PrettyPrintIM(bool aIsNewLine, const char * aFmt, ...);
void IncreaseDepth();
void DecreaseDepth();
#define PRETTY_PRINT(fmt, ...)                                                                                                     \
    do                                                                                                                             \
    {                                                                                                                              \
        PrettyPrintIM(true, fmt, ##__VA_ARGS__);                                                                                   \
    } while (0)
#define PRETTY_PRINT_SAMELINE(fmt, ...)                                                                                            \
    do                                                                                                                             \
    {                                                                                                                              \
        PrettyPrintIM(false, fmt, ##__VA_ARGS__);                                                                                  \
    } while (0)
#define PRETTY_PRINT_INCDEPTH()                                                                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        IncreaseDepth();                                                                                                           \
    } while (0)
#define PRETTY_PRINT_DECDEPTH()                                                                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        DecreaseDepth();                                                                                                           \
    } while (0)
#else
#define PRETTY_PRINT(fmt, ...)
#define PRETTY_PRINT(fmt, ...)
#define PRETTY_PRINT_SAMELINE(fmt, ...)
#define PRETTY_PRINT_INCDEPTH()
#define PRETTY_PRINT_DECDEPTH()
#endif
}; // namespace app
}; // namespace chip
