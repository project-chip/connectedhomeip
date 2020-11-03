/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          Provides implementations for the CHIP and LwIP logging functions
 *          on Qorvo platforms.
 */

#include "qvCHIP.h"

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <support/logging/CHIPLogging.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <openthread/platform/logging.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

using namespace ::chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {

/**
 * Called whenever a log message is emitted by chip or LwIP.
 *
 * This function is intended be overridden by the application to, e.g.,
 * schedule output of queued log entries.
 */
void __attribute__((weak)) OnLogOutput(void) {}

} // namespace DeviceLayer
} // namespace chip

namespace chip {
namespace Logging {

/**
 * CHIP log output function.
 */

void LogV(uint8_t module, uint8_t category, const char * msg, va_list v)
{
    if (IsCategoryEnabled(category))
    {
        char formattedMsg[CHIP_DEVICE_CONFIG_LOG_MESSAGE_MAX_SIZE];
        size_t prefixLen;

        prefixLen = 0;

        // No build-time switches in Qorvo logging module.
        // Add small prefix to show logging category for now.
        formattedMsg[prefixLen++] = '[';
        switch (category)
        {
        case kLogCategory_Error:
            formattedMsg[prefixLen++] = 'E';
            break;
        case kLogCategory_Detail:
            formattedMsg[prefixLen++] = 'D';
            break;
        case kLogCategory_Progress:
        case kLogCategory_Retain:
        default:
            formattedMsg[prefixLen++] = 'P';
            break;
        }
        formattedMsg[prefixLen++] = ']';
        formattedMsg[prefixLen++] = '[';
        GetModuleName(&formattedMsg[prefixLen], module);
        prefixLen                 = strlen(formattedMsg);
        formattedMsg[prefixLen++] = ']';
        formattedMsg[prefixLen++] = ' ';

        vsnprintf(&formattedMsg[prefixLen], sizeof(formattedMsg) - prefixLen, msg, v);

        qvCHIP_Printf(module, formattedMsg);

        // Let the application know that a log message has been emitted.
        DeviceLayer::OnLogOutput();
    }
}

} // namespace Logging
} // namespace chip

/**
 * LwIP log output function.
 */
extern "C" void LwIPLog(const char * msg, ...)
{
    char formattedMsg[CHIP_DEVICE_CONFIG_LOG_MESSAGE_MAX_SIZE];

    va_list v;

    va_start(v, msg);
    size_t len = vsnprintf(formattedMsg, sizeof(formattedMsg), msg, v);
    va_end(v);

    while (len > 0 && isspace(formattedMsg[len - 1]))
    {
        len--;
        formattedMsg[len] = 0;
    }

    qvCHIP_Printf(0x1, formattedMsg);

    // Let the application know that a log message has been emitted.
    DeviceLayer::OnLogOutput();
}

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
// Implementation taken from openthread repo - examples\platforms\qpg6095
#include "uart_qorvo.h"

extern "C" void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char * aFormat, ...)
{
    char formattedMsg[CHIP_DEVICE_CONFIG_LOG_MESSAGE_MAX_SIZE];

    va_start(v, aFormat);
    size_t len = vsnprintf(formattedMsg, sizeof(formattedMsg), aFormat, v);
    va_end(v);

    qvCHIP_Printf(0x2, formattedMsg);

    // Let the application know that a log message has been emitted.
    DeviceLayer::OnLogOutput();
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
