/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

namespace chip {
namespace app {

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

namespace {
// this is used to run in signle thread for IM message debug purpose
uint32_t gPrettyPrintingDepthLevel = 0;
char gLineBuffer[256];
size_t gCurLineBufferSize = 0;
} // namespace

class PrettyPrintCheckpoint
{
public:
    PrettyPrintCheckpoint() { mLevel = gPrettyPrintingDepthLevel; }
    ~PrettyPrintCheckpoint() { gPrettyPrintingDepthLevel = mLevel; }

private:
    uint32_t mLevel;
};
#define PRETTY_PRINT_CHECKPOINT() PrettyPrintCheckpoint lPrettyPrintCheckpoint;

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
        gPrettyPrintingDepthLevel++;                                                                                               \
    } while (0)
#define PRETTY_PRINT_DECDEPTH()                                                                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        gPrettyPrintingDepthLevel--;                                                                                               \
    } while (0)

static void PrettyPrintIM(bool aIsNewLine, const char * aFmt, ...)
{
    va_list args;
    size_t ret;
    size_t sizeLeft;

    va_start(args, aFmt);

    if (aIsNewLine)
    {
        if (gCurLineBufferSize)
        {
            // Don't need to explicitly NULL-terminate the string because
            // snprintf takes care of that.
            ChipLogDetail(DataManagement, "%s", gLineBuffer);
            gCurLineBufferSize = 0;
        }

        for (uint32_t i = 0; i < gPrettyPrintingDepthLevel; i++)
        {
            if (sizeof(gLineBuffer) > gCurLineBufferSize)
            {
                sizeLeft = sizeof(gLineBuffer) - gCurLineBufferSize;
                ret      = (size_t)(snprintf(gLineBuffer + gCurLineBufferSize, sizeLeft, "\t"));
                if (ret > 0)
                {
                    gCurLineBufferSize += std::min(ret, sizeLeft);
                }
            }
        }
    }

    if (sizeof(gLineBuffer) > gCurLineBufferSize)
    {
        sizeLeft = sizeof(gLineBuffer) - gCurLineBufferSize;
        ret      = (size_t)(vsnprintf(gLineBuffer + gCurLineBufferSize, sizeLeft, aFmt, args));
        if (ret > 0)
        {
            gCurLineBufferSize += std::min(ret, sizeLeft);
        }
    }

    va_end(args);
}
#else
#define PRETTY_PRINT_CHECKPOINT()
#define PRETTY_PRINT(fmt, ...)
#define PRETTY_PRINT(fmt, ...)
#define PRETTY_PRINT_SAMELINE(fmt, ...)
#define PRETTY_PRINT_INCDEPTH()
#define PRETTY_PRINT_DECDEPTH()
#endif
}; // namespace app
}; // namespace chip
