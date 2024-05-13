/* See Project CHIP LICENSE file for licensing information. */
#include <platform/logging/LogV.h>

#include "qvCHIP.h"

#include <lib/core/CHIPConfig.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <lib/support/EnforceFormat.h>
#include <lib/support/logging/Constants.h>
#include <platform/CHIPDeviceConfig.h>
#include <system/SystemClock.h>

#include <cstdio>
#include <ctype.h>
#include <string.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <openthread/platform/logging.h>
#include <openthread/platform/memory.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

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
 * @brief Add a timestamp in hh:mm:ss.ms format and the given prefix string to the given char buffer
 * The time stamp is derived from the boot time
 *
 * @param logBuffer: pointer to the buffer where to add the information
 *        prefix: A prefix to add to the trace e.g. The category
 *        maxSize: Space availaible in the given buffer.
 */
static size_t AddTimeStampAndPrefixStr(char * logBuffer, const char * prefix, size_t maxSize)
{
    // Derive the hours, minutes, seconds and milliseconds since boot time millisecond counter
    uint64_t bootTime     = chip::System::SystemClock().GetMonotonicMilliseconds64().count();
    uint16_t milliseconds = bootTime % 1000;
    uint32_t totalSeconds = bootTime / 1000;
    uint8_t seconds       = totalSeconds % 60;
    totalSeconds /= 60;
    uint8_t minutes = totalSeconds % 60;
    uint32_t hours  = totalSeconds / 60;

    return snprintf(logBuffer, maxSize, "[%02lu:%02u:%02u.%03u]%s", hours, minutes, seconds, milliseconds, prefix);
}

/**
 * CHIP log output function.
 */

void ENFORCE_FORMAT(3, 0) LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
    size_t formattedMsgLen;

    // No build-time switches in Qorvo logging module.
    // Add small prefix to show logging category for now.
    switch (category)
    {
    case kLogCategory_Error:
        formattedMsgLen = AddTimeStampAndPrefixStr(formattedMsg, "[E]", CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE);
        break;
    case kLogCategory_Detail:
        formattedMsgLen = AddTimeStampAndPrefixStr(formattedMsg, "[D]", CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE);
        break;
    case kLogCategory_Progress:
    default:
        formattedMsgLen = AddTimeStampAndPrefixStr(formattedMsg, "[P]", CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE);
        break;
    }
    snprintf(formattedMsg + formattedMsgLen, sizeof(formattedMsg) - formattedMsgLen, "[%s] ", module);
    formattedMsg[sizeof(formattedMsg) - 2] = 0; // -2 to allow at least one char for the vsnprintf
    formattedMsgLen                        = strlen(formattedMsg);

    vsnprintf(formattedMsg + formattedMsgLen, sizeof(formattedMsg) - formattedMsgLen, msg, v);

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
