/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Texas Instruments Incorporated
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
 *          for the Texas Instruments CC1352 platform. This uses one of the
 *          UARTs configured with SysConfig. Future implementations may use
 *          ITM.
 *
 */

#include "ti_drivers_config.h"
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <support/logging/CHIPLogging.h>

#include <ti/drivers/UART.h>

#include <stdio.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;

#define DEVICE_LAYER_LOG_BUFFER_SIZE (256)
UART_Handle sDebugUartHandle;
char sDebugUartBuffer[DEVICE_LAYER_LOG_BUFFER_SIZE];

extern "C" int cc13x2_26x2LogInit(void)
{
    UART_Params uartParams;

    UART_init();

    UART_Params_init(&uartParams);
    // Most params can be default because we only send data, we don't receive
    uartParams.baudRate = 115200;
    // unclear why the UART driver sticks in writing sometimes
    uartParams.writeTimeout = 10000; // ticks

    sDebugUartHandle = UART_open(CONFIG_UART_DEBUG, &uartParams);
    return 0;
}

extern "C" void cc13x2_26x2VLog(const char * msg, va_list v)
{
    int ret;

    ret = vsnprintf(sDebugUartBuffer, sizeof(sDebugUartBuffer), msg, v);
    if (0 < ret)
    {
        // PuTTY likes \r\n
        size_t len                = (ret + 2U) < sizeof(sDebugUartBuffer) ? (ret + 2) : sizeof(sDebugUartBuffer);
        sDebugUartBuffer[len - 2] = '\r';
        sDebugUartBuffer[len - 1] = '\n';
        sDebugUartBuffer[len]     = '\0';

        UART_write(sDebugUartHandle, sDebugUartBuffer, len);
    }
}

namespace chip {
namespace DeviceLayer {

/**
 * Called whenever a log message is emitted.
 *
 * Can be overriden by the device logging file
 */
void __attribute__((weak)) OnLogOutput(void) {}

} // namespace DeviceLayer
} // namespace chip

namespace chip {
namespace Logging {

void LogV(uint8_t module, uint8_t category, const char * msg, va_list v)
{
    (void) module;
    (void) category;

    cc13x2_26x2VLog(msg, v);

    DeviceLayer::OnLogOutput();
}

} // namespace Logging
} // namespace chip

/**
 * LwIP log output function.
 */
extern "C" void LwIPLog(const char * msg, ...)
{
    va_list v;

    va_start(v, msg);

    cc13x2_26x2VLog(msg, v);

    DeviceLayer::OnLogOutput();
    va_end(v);
}

/**
 * Platform log output function.
 */
extern "C" void cc13x2_26x2Log(const char * msg, ...)
{
    va_list v;

    va_start(v, msg);

    cc13x2_26x2VLog(msg, v);

    DeviceLayer::OnLogOutput();
    va_end(v);
}

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
extern "C" void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char * aFormat, ...)
{
    va_list v;

    (void) aLogLevel;
    (void) aLogRegion;

    va_start(v, aFormat);

    cc13x2_26x2VLog(aFormat, v);

    DeviceLayer::OnLogOutput();
    va_end(v);
}
#endif
