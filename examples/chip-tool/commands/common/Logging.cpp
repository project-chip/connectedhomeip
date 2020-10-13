/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include <support/CHIPLogging.h>
#include <support/CodeUtils.h>

#include <stdlib.h>
#include <string.h>

using namespace ::chip::Logging;

constexpr LogCategory kDefaultLoggingLevel = kLogCategory_Detail;

void ConfigureChipLogging()
{
    LogCategory chipLogLevel = kDefaultLoggingLevel;

    const char * level = getenv("CHIP_LOG_LEVEL");
    VerifyOrExit(level != NULL, /**/);

    if (strcmp(level, "none") == 0)
    {
        chipLogLevel = kLogCategory_None;
    }
    else if (strcmp(level, "error") == 0)
    {
        chipLogLevel = kLogCategory_Error;
    }
    else if (strcmp(level, "progress") == 0)
    {
        chipLogLevel = kLogCategory_Progress;
    }
    else if (strcmp(level, "detail") == 0)
    {
        chipLogLevel = kLogCategory_Detail;
    }
    else if (strcmp(level, "retain") == 0)
    {
        chipLogLevel = kLogCategory_Retain;
    }

exit:
    SetLogFilter(chipLogLevel);
}
