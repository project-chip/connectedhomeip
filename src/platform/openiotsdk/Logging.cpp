/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 *          Provides an implementation of logging support
 *          for Open IOT SDK platform.
 */

#include <lib/core/CHIPConfig.h>
#include <lib/support/EnforceFormat.h>
#include <lib/support/logging/CHIPLogging.h>
#include <lib/support/logging/Constants.h>
#include <platform/logging/LogV.h>

#include <stdio.h>
#include <string.h>

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
void ENFORCE_FORMAT(3, 0) LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    size_t prefixLen = 0;
    snprintf(logMsgBuffer, sizeof(logMsgBuffer), "[%s]", module);
    logMsgBuffer[sizeof(logMsgBuffer) - 2] = 0; // -2 to allow at least one char for the vsnprintf
    prefixLen                              = strlen(logMsgBuffer);
    vsnprintf(logMsgBuffer + prefixLen, sizeof(logMsgBuffer) - prefixLen, msg, v);

    // TODO

    // Let the application know that a log message has been emitted.
    DeviceLayer::OnLogOutput();
}

void openiotsdk_logging_init()
{
    // TODO
}

} // namespace Platform
} // namespace Logging
} // namespace chip
