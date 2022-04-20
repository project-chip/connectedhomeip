/* See Project CHIP LICENSE file for licensing information. */
#include <platform/logging/LogV.h>

#include "qvCHIP.h"

#include <lib/core/CHIPConfig.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <lib/support/EnforceFormat.h>
#include <lib/support/logging/Constants.h>
#include <platform/CHIPDeviceConfig.h>

#include <cstdio>
#include <ctype.h>
#include <string.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <openthread/platform/logging.h>
#include <openthread/platform/memory.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

#ifdef PW_RPC_ENABLED
#include "PigweedLogger.h"
#endif

constexpr uint8_t kPrintfModuleLwip       = 0x01;
constexpr uint8_t kPrintfModuleOpenThread = 0x02;
constexpr uint8_t kPrintfModuleLogging    = 0x03;

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
namespace Platform {

/**
 * CHIP log output function.
 */

void ENFORCE_FORMAT(3, 0) LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
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
    default:
        formattedMsg[prefixLen++] = 'P';
        break;
    }
    snprintf(formattedMsg + prefixLen, sizeof(formattedMsg) - prefixLen, "][%s] ", module);
    formattedMsg[sizeof(formattedMsg) - 2] = 0; // -2 to allow at least one char for the vsnprintf
    prefixLen                              = strlen(formattedMsg);

    vsnprintf(formattedMsg + prefixLen, sizeof(formattedMsg) - prefixLen, msg, v);

    qvCHIP_Printf(kPrintfModuleLogging, formattedMsg);

    // Let the application know that a log message has been emitted.
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
    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];

    va_list v;

    va_start(v, msg);
    size_t len = vsnprintf(formattedMsg, sizeof(formattedMsg), msg, v);
    va_end(v);

    while (len > 0 && isspace(formattedMsg[len - 1]))
    {
        len--;
        formattedMsg[len] = 0;
    }

    qvCHIP_Printf(kPrintfModuleLwip, formattedMsg);

    // Let the application know that a log message has been emitted.
    chip::DeviceLayer::OnLogOutput();
}
#endif // #if CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
extern "C" void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char * aFormat, ...)
{
    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];

    va_list v;

    va_start(v, aFormat);
    vsnprintf(formattedMsg, sizeof(formattedMsg), aFormat, v);
    va_end(v);

    qvCHIP_Printf(kPrintfModuleOpenThread, formattedMsg);

    // Let the application know that a log message has been emitted.
    chip::DeviceLayer::OnLogOutput();
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
