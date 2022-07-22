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
 *          for the Texas Instruments CC32XX platform. This uses one of the
 *          UARTs configured with SysConfig. Future implementations may use
 *          ITM.
 *
 */

#include "ti_drivers_config.h"
#include <lib/support/logging/CHIPLogging.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <pthread.h>
#include <ti/drivers/UART2.h>

#include <stdio.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;

#define DEVICE_LAYER_LOG_BUFFER_SIZE (256)
static UART2_Handle sDebugUartHandle;
static char sDebugUartBuffer[DEVICE_LAYER_LOG_BUFFER_SIZE];
// static pthread_mutex_t mutex;

extern "C" int cc32xxLogInit(void)
{
    UART2_Params uartParams;

    UART2_Params_init(&uartParams);

    uartParams.baudRate = 115200;
    sDebugUartHandle    = UART2_open(CONFIG_UART2_0, &uartParams);

    /* Remove uart receive from LPDS dependency */
    UART2_rxDisable(sDebugUartHandle);

    /* Enable debug mutex */
    // pthread_mutex_init(&mutex, NULL);
    return 0;
}

extern "C" void cc32xxVLog(const char * msg, va_list v)
{
    int ret;

    // pthread_mutex_lock(&mutex);
    ret = vsnprintf(sDebugUartBuffer, sizeof(sDebugUartBuffer), msg, v);
    if (0 < ret)
    {
        // PuTTY likes \r\n
        size_t len                = (ret + 2U) < sizeof(sDebugUartBuffer) ? (ret + 2) : sizeof(sDebugUartBuffer);
        sDebugUartBuffer[len - 2] = '\r';
        sDebugUartBuffer[len - 1] = '\n';
        sDebugUartBuffer[len]     = '\0';

        UART2_write(sDebugUartHandle, sDebugUartBuffer, len, &len);
    }
    // pthread_mutex_unlock(&mutex);
}

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

void LogV(const char * module_name, uint8_t category, const char * msg, va_list v)
{
    (void) module_name;
    (void) category;

    cc32xxVLog(msg, v);

    DeviceLayer::OnLogOutput();
}

void LogV(uint8_t module, uint8_t category, const char * msg, va_list v)
{
    (void) module;
    (void) category;

    cc32xxVLog(msg, v);

    DeviceLayer::OnLogOutput();
}

} // namespace Platform
} // namespace Logging
} // namespace chip

/**
 * LwIP log output function.
 */
extern "C" void LwIPLog(const char * msg, ...)
{
    va_list v;

    va_start(v, msg);

    cc32xxVLog(msg, v);

    DeviceLayer::OnLogOutput();
    va_end(v);
}

/**
 * Platform log output function.
 */
extern "C" void cc32xxLog(const char * msg, ...)
{
    va_list v;

    va_start(v, msg);

    cc32xxVLog(msg, v);

    DeviceLayer::OnLogOutput();
    va_end(v);
}

#
