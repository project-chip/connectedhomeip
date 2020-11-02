/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          on Nordic nRF52 platforms.
 */

#define NRF_LOG_MODULE_NAME chip

#include <FreeRTOS.h>
#include <nrf_log.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <support/logging/CHIPLogging.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <openthread/platform/logging.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

// If deferred logging is NOT enabled, do bother calling the nRF5 SDK nrf_log_push() function.
#if !NRF_LOG_DEFERRED
#undef NRF_LOG_PUSH
#define NRF_LOG_PUSH(x) (x)
#endif

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

NRF_LOG_MODULE_REGISTER();

namespace chip {
namespace Logging {

/**
 * CHIP log output function.
 */

void LogV(uint8_t module, uint8_t category, const char * msg, va_list v)
{
    (void) module;
    (void) category;

#if NRF_LOG_ENABLED

    if (IsCategoryEnabled(category))
    {
        {
            char formattedMsg[CHIP_DEVICE_CONFIG_LOG_MESSAGE_MAX_SIZE];
            size_t prefixLen;

            constexpr size_t maxPrefixLen = ChipLoggingModuleNameLen + 3;
            static_assert(sizeof(formattedMsg) > maxPrefixLen);

            // Form the log prefix, e.g. "[DL] "
            formattedMsg[0] = '[';
            GetModuleName(formattedMsg + 1, module);
            prefixLen                 = strlen(formattedMsg);
            formattedMsg[prefixLen++] = ']';
            formattedMsg[prefixLen++] = ' ';

            // Append the log message.
            vsnprintf(formattedMsg + prefixLen, sizeof(formattedMsg) - prefixLen, msg, v);

            // Invoke the NRF logging library to log the message.
            switch (category)
            {
            case kLogCategory_Error:
                NRF_LOG_ERROR("%s", NRF_LOG_PUSH(formattedMsg));
                break;
            case kLogCategory_Progress:
            case kLogCategory_Retain:
            default:
                NRF_LOG_INFO("%s", NRF_LOG_PUSH(formattedMsg));
                break;
            case kLogCategory_Detail:
                NRF_LOG_DEBUG("%s", NRF_LOG_PUSH(formattedMsg));
                break;
            }

#if configCHECK_FOR_STACK_OVERFLOW
            // Force a stack overflow check.
            if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
                taskYIELD();
#endif
        }

        // Let the application know that a log message has been emitted.
        DeviceLayer::OnLogOutput();
    }
#else  // NRF_LOG_ENABLED
    (void) msg;
    (void) v;
#endif // NRF_LOG_ENABLED
}

} // namespace Logging
} // namespace chip

#undef NRF_LOG_MODULE_NAME
#define NRF_LOG_MODULE_NAME lwip
NRF_LOG_MODULE_REGISTER();

/**
 * LwIP log output function.
 */
extern "C" void LwIPLog(const char * msg, ...)
{
    va_list v;

    va_start(v, msg);

#if NRF_LOG_ENABLED
    {
        char formattedMsg[CHIP_DEVICE_CONFIG_LOG_MESSAGE_MAX_SIZE];

        // Append the log message.
        size_t len = vsnprintf(formattedMsg, sizeof(formattedMsg), msg, v);

        while (len > 0 && isspace(formattedMsg[len - 1]))
        {
            len--;
            formattedMsg[len] = 0;
        }

        // Invoke the NRF logging library to log the message.
        NRF_LOG_DEBUG("%s", NRF_LOG_PUSH(formattedMsg));
    }

    // Let the application know that a log message has been emitted.
    DeviceLayer::OnLogOutput();

#endif // NRF_LOG_ENABLED

    va_end(v);
}

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

#undef NRF_LOG_MODULE_NAME
#define NRF_LOG_MODULE_NAME thread
NRF_LOG_MODULE_REGISTER();

extern "C" void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char * aFormat, ...)
{
    va_list v;

    (void) aLogLevel;
    (void) aLogRegion;

    va_start(v, aFormat);

#if NRF_LOG_ENABLED
    {
        char formattedMsg[CHIP_DEVICE_CONFIG_LOG_MESSAGE_MAX_SIZE];

        // Append the log message.
        vsnprintf(formattedMsg, sizeof(formattedMsg), aFormat, v);

        // Invoke the NRF logging library to log the message.
        switch (aLogLevel)
        {
        case OT_LOG_LEVEL_CRIT:
            NRF_LOG_ERROR("%s", NRF_LOG_PUSH(formattedMsg));
            break;
        case OT_LOG_LEVEL_WARN:
            NRF_LOG_WARNING("%s", NRF_LOG_PUSH(formattedMsg));
            break;
        case OT_LOG_LEVEL_NOTE:
        case OT_LOG_LEVEL_INFO:
        default:
            NRF_LOG_INFO("%s", NRF_LOG_PUSH(formattedMsg));
            break;
        case OT_LOG_LEVEL_DEBG:
            NRF_LOG_DEBUG("%s", NRF_LOG_PUSH(formattedMsg));
            break;
        }

#if configCHECK_FOR_STACK_OVERFLOW
        // Force a stack overflow check.
        if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
            taskYIELD();
#endif
    }

    // Let the application know that a log message has been emitted.
    DeviceLayer::OnLogOutput();

#endif // NRF_LOG_ENABLED

    va_end(v);
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
