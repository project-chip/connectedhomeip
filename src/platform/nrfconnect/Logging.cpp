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
 *          Provides implementations for the CHIP logging functions
 *          on Nordic nRF Connect SDK platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <support/logging/CHIPLogging.h>

#include <logging/log.h>

#include <cstdio>

using namespace ::chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;

LOG_MODULE_REGISTER(chip, LOG_LEVEL_DBG);

namespace {

void GetModuleName(char * buf, uint8_t module)
{
    if (module == ::chip::Logging::kLogModule_DeviceLayer)
    {
        memcpy(buf, "DL", 3);
    }
    else
    {
        ::chip::Logging::GetModuleName(buf, module);
    }
}

} // unnamed namespace

namespace chip {
namespace DeviceLayer {

/**
 * Called whenever a log message is emitted by chip.
 *
 * This function is intended be overridden by the application to, e.g.,
 * schedule output of queued log entries.
 */
void __attribute__((weak)) OnLogOutput(void) {}

} // namespace DeviceLayer

namespace Logging {

/**
 * CHIP log output function.
 */

void LogV(uint8_t module, uint8_t category, const char * msg, va_list v)
{
    if (!IsCategoryEnabled(category))
        return;

    {
        char formattedMsg[CHIP_DEVICE_CONFIG_LOG_MESSAGE_MAX_SIZE];
        size_t prefixLen;

        constexpr size_t maxPrefixLen = ChipLoggingModuleNameLen + 3;
        static_assert(sizeof(formattedMsg) > maxPrefixLen);

        // Form the log prefix, e.g. "[DL] "
        formattedMsg[0] = '[';
        ::GetModuleName(formattedMsg + 1, module);
        prefixLen                 = strlen(formattedMsg);
        formattedMsg[prefixLen++] = ']';
        formattedMsg[prefixLen++] = ' ';

        // Append the log message.
        vsnprintf(formattedMsg + prefixLen, sizeof(formattedMsg) - prefixLen, msg, v);

        // Invoke the Zephyr logging library to log the message.
        switch (category)
        {
        case kLogCategory_Error:
            LOG_ERR("%s", log_strdup(formattedMsg));
            break;
        case kLogCategory_Progress:
        case kLogCategory_Retain:
        default:
            LOG_INF("%s", log_strdup(formattedMsg));
            break;
        case kLogCategory_Detail:
            LOG_DBG("%s", log_strdup(formattedMsg));
            break;
        }
    }

    // Let the application know that a log message has been emitted.
    DeviceLayer::OnLogOutput();
}

} // namespace Logging
} // namespace chip
