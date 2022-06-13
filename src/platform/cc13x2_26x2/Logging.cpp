/* See Project CHIP LICENSE file for licensing information. */

#include <platform/logging/LogV.h>

#include <lib/core/CHIPConfig.h>
#include <platform/CHIPDeviceConfig.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <openthread/platform/logging.h>
#endif

#include "ti_drivers_config.h"

#include <ti/drivers/UART.h>

#include <stdio.h>

UART_Handle sDebugUartHandle;
char sDebugUartBuffer[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];

#if MATTER_CC13X2_26X2_PLATFORM_LOG_ENABLED
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

        UART_write(sDebugUartHandle, sDebugUartBuffer, len);
    }
}

#else

/* log functins defined somewhere else */
extern "C" int cc13x2_26x2LogInit(void);
extern "C" void cc13x2_26x2VLog(const char * msg, va_list v);

#endif // MATTER_CC13X2_26X2_PLATFORM_LOG_ENABLED

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

    cc13x2_26x2VLog(msg, v);

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

    cc13x2_26x2VLog(msg, v);

    chip::DeviceLayer::OnLogOutput();
    va_end(v);
}
#endif // #if CHIP_SYSTEM_CONFIG_USE_LWIP

/**
 * Platform log output function.
 */
extern "C" void cc13x2_26x2Log(const char * msg, ...)
{
    va_list v;

    va_start(v, msg);

    cc13x2_26x2VLog(msg, v);

    chip::DeviceLayer::OnLogOutput();
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

    chip::DeviceLayer::OnLogOutput();
    va_end(v);
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
