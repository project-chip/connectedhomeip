/*
 *    Copyright (c) 2020 Project CHIP Authors
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
extern "C" void assert(int condition)
{
    if (condition == 0)
    {
        for (;;)
            ;
    }
}

extern "C" void ASSERT_GENERAL(int condition)
{
    if (condition == 0)
    {
        for (;;)
            ;
    }
}

extern "C" void hostapd_parse_ip_addr(void)
{
    for (;;)
        ;
}
extern "C" void _link(void)
{
    for (;;)
        ;
}
extern "C" void _unlink(void)
{
    for (;;)
        ;
}

/* See Project CHIP LICENSE file for licensing information. */

#include <platform/logging/LogV.h>

#include <lib/core/CHIPConfig.h>
#include <platform/CHIPDeviceConfig.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <openthread/platform/logging.h>
#endif

#include "ti_drivers_config.h"

#include <ti/drivers/UART2.h>

#include <stdio.h>

UART2_Handle sDebugUartHandle;
char sDebugUartBuffer[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];

#if MATTER_CC35XX_PLATFORM_LOG_ENABLED
extern "C" int cc35xxLogInit(void)
{
    UART2_Params uartParams;

    UART2_Params_init(&uartParams);
    // Most params can be default because we only send data, we don't receive
    uartParams.baudRate = 115200;

    sDebugUartHandle = UART2_open(CONFIG_UART2_0, &uartParams);
    return 0;
}

extern "C" void cc35xxVLog(const char * msg, va_list v)
{
    int ret;

    ret = vsnprintf(sDebugUartBuffer, sizeof(sDebugUartBuffer), msg, v);
    if (0 < ret)
    {
        // PuTTY likes \r\n
        size_t len                = (ret + 2U) < sizeof(sDebugUartBuffer) ? (ret + 2) : sizeof(sDebugUartBuffer);
        sDebugUartBuffer[len - 2] = '\r';
        sDebugUartBuffer[len - 1] = '\n';

        UART2_write(sDebugUartHandle, sDebugUartBuffer, len, NULL);
    }
}

#else

/* log functins defined somewhere else */
extern "C" int cc35xxLogInit(void);
extern "C" void cc35xxVLog(const char * msg, va_list v);

#endif // MATTER_CC35XX_PLATFORM_LOG_ENABLED

namespace chip {
namespace DeviceLayer {

/**
 * Called whenever a log message is emitted.
 *
 * Can be overridden by the device logging file
 */
void __attribute__((weak)) OnLogOutput(void) {}

} // namespace DeviceLayer
} // namespace chip

namespace chip {
namespace Logging {
namespace Platform {

void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    (void) module;
    (void) category;

    cc35xxVLog(msg, v);

    chip::DeviceLayer::OnLogOutput();
}

} // namespace Platform
} // namespace Logging
} // namespace chip

#if CHIP_SYSTEM_CONFIG_USE_LWIP
/**
 * LwIP log output function.
 */
extern "C" void LwIPLog(const char * msg, ...)
{
    va_list v;

    va_start(v, msg);

    cc35xxVLog(msg, v);

    chip::DeviceLayer::OnLogOutput();
    va_end(v);
}
#endif // #if CHIP_SYSTEM_CONFIG_USE_LWIP

/**
 * Platform log output function.
 */
extern "C" void cc35xxLog(const char * msg, ...)
{
    va_list v;

    va_start(v, msg);

    cc35xxVLog(msg, v);

    chip::DeviceLayer::OnLogOutput();
    va_end(v);
}

extern "C" int Report(const char * pcFormat, ...)
{
    va_list v;

    va_start(v, pcFormat);

    cc35xxVLog(pcFormat, v);

    chip::DeviceLayer::OnLogOutput();
    va_end(v);
    return 0;
}
