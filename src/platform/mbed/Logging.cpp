/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *          Logging implementation for Mbed platform
 */

#include <lib/core/CHIPConfig.h>
#include <lib/support/logging/CHIPLogging.h>
#include <lib/support/logging/Constants.h>
#include <platform/logging/LogV.h>

#include <stdio.h>
#include <string.h>

#include "mbed-trace/mbed_trace.h"

#ifdef MBED_TRACE_FILTER_LENGTH
#define DEFAULT_TRACE_FILTER_LENGTH MBED_TRACE_FILTER_LENGTH
#else
#define DEFAULT_TRACE_FILTER_LENGTH 24
#endif

static const char * TRACE_GROUP = "CHIP";

namespace chip {
namespace DeviceLayer {

/**
 * Called whenever a log message is emitted by chip or LwIP.
 *
 * This function is intended be overridden by the application to, e.g.,
 * schedule output of queued log entries.
 */
void __attribute__((weak)) OnLogOutput() {}

} // namespace DeviceLayer

namespace Logging {
namespace Platform {

/**
 * Logging static buffer
 */
namespace {
char logMsgBuffer[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
}

/**
 * CHIP log output functions.
 */
void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    size_t prefixLen = 0;
    snprintf(logMsgBuffer, sizeof(logMsgBuffer), "[%s]", module);
    logMsgBuffer[sizeof(logMsgBuffer) - 2] = 0; // -2 to allow at least one char for the vsnprintf
    prefixLen                              = strlen(logMsgBuffer);
    vsnprintf(logMsgBuffer + prefixLen, sizeof(logMsgBuffer) - prefixLen, msg, v);

    switch (category)
    {
    case kLogCategory_Error:
        tr_err("%s", logMsgBuffer);
        break;
    case kLogCategory_Progress:
        tr_info("%s", logMsgBuffer);
        break;
    case kLogCategory_Detail:
    default:
        tr_debug("%s", logMsgBuffer);
        break;
    }

    // Let the application know that a log message has been emitted.
    DeviceLayer::OnLogOutput();
}

void mbed_logging_init()
{
    // Mbed trace initalization
    mbed_trace_init();
    mbed_trace_include_filters_set((char *) TRACE_GROUP);
    mbed_trace_config_set(TRACE_ACTIVE_LEVEL_ALL | TRACE_MODE_COLOR);

#if MBED_BSD_SOCKET_TRACE
    char filtersStr[DEFAULT_TRACE_FILTER_LENGTH];
    auto filters = mbed_trace_include_filters_get();
    snprintf(filtersStr, sizeof(filtersStr), "%s,BSDS,NETS", filters);
    mbed_trace_include_filters_set(filtersStr);
#endif

#if MBED_DEBUG
    chip::Logging::SetLogFilter(chip::Logging::LogCategory::kLogCategory_Detail);
#else
    chip::Logging::SetLogFilter(chip::Logging::LogCategory::kLogCategory_Progress);
#endif
}

} // namespace Platform
} // namespace Logging
} // namespace chip
