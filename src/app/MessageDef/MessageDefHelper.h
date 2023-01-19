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

// We need CHIPLogging.h to get the right value for CHIP_DETAIL_LOGGING here.
#include <lib/support/logging/CHIPLogging.h>

#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_PRETTY_PRINT && CHIP_DETAIL_LOGGING
/**
 * Start a new "blank" line.  This will actually print out whitespace to the
 * current indent level, which can be followed with PRETTY_PRINT_SAMELINE calls.
 */
void PrettyPrintIMBlankLine();
void PrettyPrintIM(bool aIsNewLine, const char * aFmt, ...) ENFORCE_FORMAT(2, 3);
void IncreaseDepth();
void DecreaseDepth();
#define PRETTY_PRINT_BLANK_LINE()                                                                                                  \
    do                                                                                                                             \
    {                                                                                                                              \
        PrettyPrintIMBlankLine();                                                                                                  \
    } while (0)
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
#define PRETTY_PRINT_BLANK_LINE()
#define PRETTY_PRINT(fmt, ...)
#define PRETTY_PRINT(fmt, ...)
#define PRETTY_PRINT_SAMELINE(fmt, ...)
#define PRETTY_PRINT_INCDEPTH()
#define PRETTY_PRINT_DECDEPTH()
#endif

#if CHIP_CONFIG_IM_PRETTY_PRINT
// Parse an IM payload (attribute value, command fields, event fields,
// recursively parsing any complex types encountered.
CHIP_ERROR CheckIMPayload(TLV::TLVReader & aReader, int aDepth, const char * aLabel);
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

}; // namespace app
}; // namespace chip
