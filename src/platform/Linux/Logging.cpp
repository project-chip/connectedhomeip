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
 *          on Linux platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <support/logging/CHIPLogging.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <openthread/platform/logging.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

#include <assert.h>
#include <stdarg.h>
#include <syslog.h>

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
 * Openchip log output function.
 */
void Log(uint8_t module, uint8_t category, const char * msg, ...)
{
    va_list v;
    va_start(v, msg);

    if (IsCategoryEnabled(category))
    {
        vsyslog(LOG_INFO, msg, v);

        // Let the application know that a log message has been emitted.
        DeviceLayer::OnLogOutput();
    }

    va_end(v);
}

} // namespace Logging
} // namespace chip

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

extern "C" void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char * aFormat, ...)
{
    OT_UNUSED_VARIABLE(aLogRegion);

    va_list args;

    switch (aLogLevel)
    {
    case OT_LOG_LEVEL_NONE:
        aLogLevel = LOG_ALERT;
        break;
    case OT_LOG_LEVEL_CRIT:
        aLogLevel = LOG_CRIT;
        break;
    case OT_LOG_LEVEL_WARN:
        aLogLevel = LOG_WARNING;
        break;
    case OT_LOG_LEVEL_NOTE:
        aLogLevel = LOG_NOTICE;
        break;
    case OT_LOG_LEVEL_INFO:
        aLogLevel = LOG_INFO;
        break;
    case OT_LOG_LEVEL_DEBG:
        aLogLevel = LOG_DEBUG;
        break;
    default:
        assert(false);
        aLogLevel = LOG_DEBUG;
        break;
    }

    va_start(args, aFormat);
    vsyslog(aLogLevel, aFormat, args);
    va_end(args);
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
