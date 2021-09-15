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

#include "MessageDefHelper.h"
#include <algorithm>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppBuildConfig.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK && CHIP_DETAIL_LOGGING
// this is used to run in signle thread for IM message debug purpose
namespace {
uint32_t gPrettyPrintingDepthLevel = 0;
char gLineBuffer[256];
size_t gCurLineBufferSize = 0;
} // namespace

void PrettyPrintIM(bool aIsNewLine, const char * aFmt, ...)
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
void IncreaseDepth()
{
    gPrettyPrintingDepthLevel++;
}

void DecreaseDepth()
{
    gPrettyPrintingDepthLevel--;
}
#endif
}; // namespace app
}; // namespace chip
