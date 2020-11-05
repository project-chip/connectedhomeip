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

#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include <inttypes.h>
#include <sstream>
#include <stdlib.h>
#include <string.h>

using namespace ::chip::Logging;

constexpr LogCategory kDefaultLoggingLevel = kLogCategory_Detail;
constexpr uint16_t kDefaultRemotePort = CHIP_PORT;
constexpr uint16_t kDefaultLocalPort = CHIP_PORT;

void ConfigureChipLogging()
{
    LogCategory chipLogLevel = kDefaultLoggingLevel;

    const char * level = getenv("CHIP_TOOL_LOG_LEVEL");
    VerifyOrExit(level != NULL, /**/);

    if (strcasecmp(level, "none") == 0)
    {
        chipLogLevel = kLogCategory_None;
    }
    else if (strcasecmp(level, "error") == 0)
    {
        chipLogLevel = kLogCategory_Error;
    }
    else if (strcasecmp(level, "progress") == 0)
    {
        chipLogLevel = kLogCategory_Progress;
    }
    else if (strcasecmp(level, "detail") == 0)
    {
        chipLogLevel = kLogCategory_Detail;
    }
    else if (strcasecmp(level, "retain") == 0)
    {
        chipLogLevel = kLogCategory_Retain;
    }

exit:
    SetLogFilter(chipLogLevel);
}

uint16_t GetPort(uint16_t defaultPortValue, const char * name)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint16_t port = defaultPortValue;
    std::stringstream ss;

    const char * portStr = getenv(name);
    VerifyOrExit(portStr != NULL, /**/);

    ss.str(portStr);
    ss >> port;
    VerifyOrExit(ss.fail() || ss.eof(), err = CHIP_ERROR_INVALID_ARGUMENT);

exit:
  if (err != CHIP_NO_ERROR)
  {
      ChipLogError(chipTool, "Invalid %s value: %s. Using: %" PRIu16, name, portStr, defaultPortValue);
      port = defaultPortValue;
  }

  return port;
}

uint16_t GetRemotePort()
{
    return GetPort(kDefaultRemotePort, "CHIP_TOOL_REMOTE_PORT");
}

uint16_t GetLocalPort()
{
    return GetPort(kDefaultLocalPort, "CHIP_TOOL_LOCAL_PORT");
}
