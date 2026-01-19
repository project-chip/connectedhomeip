/* See Project CHIP LICENSE file for licensing information. */
#include <platform/silabs/Logging.h>

#include <platform/logging/LogV.h>

#include <lib/core/CHIPConfig.h>
#include <platform/CHIPDeviceConfig.h>

#include <lib/support/SafeString.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemClock.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <openthread/platform/logging.h>
#endif

#include <stdio.h>
#include <string.h>

#ifdef PW_RPC_ENABLED
#include "PigweedLogger.h"
#endif

// RTT Buffer size and name
#ifndef LOG_RTT_BUFFER_INDEX
#define LOG_RTT_BUFFER_INDEX 0
#endif

/**
 * @def LOG_RTT_BUFFER_NAME
 *
 * RTT's name. Only used if LOG_RTT_BUFFER_INDEX is not 0. Otherwise,
 * the buffer name is fixed to "Terminal".
 *
 */
#ifndef LOG_RTT_BUFFER_NAME
#define LOG_RTT_BUFFER_NAME "Terminal"
#endif

/**
 * @def LOG_RTT_BUFFER_SIZE
 *
 * LOG RTT's buffer size. Only used if LOG_RTT_BUFFER_INDEX is not 0. To
 * configure buffer #0 size, check the BUFFER_SIZE_UP definition in
 * SEGGER_RTT_Conf.h
 *
 */
#ifndef LOG_RTT_BUFFER_SIZE
#define LOG_RTT_BUFFER_SIZE 256
#endif

#if SILABS_LOG_OUT_UART
#include "uart.h"
#endif

// SEGGER_RTT includes
#if !SILABS_LOG_OUT_UART
#include "SEGGER_RTT.h"
#include "SEGGER_RTT_Conf.h"
#endif

using namespace chip::Logging::Platform;

#if SILABS_LOG_ENABLED
static bool sLogInitialized = false;
#endif
#if LOG_RTT_BUFFER_INDEX != 0
static uint8_t sLogBuffer[LOG_RTT_BUFFER_SIZE];
static uint8_t sCmdLineBuffer[LOG_RTT_BUFFER_SIZE];
#endif

#if SILABS_LOG_ENABLED

using namespace chip;

// Forward declaration
#if !SILABS_LOG_OUT_UART
static void PrintLog(const char * msg);
#endif // !SILABS_LOG_OUT_UART

namespace chip {
namespace DeviceLayer {

/**
 * Called whenever a log message is emitted by Chip or LwIP.
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

size_t AddTimeStampAndPrefixStr(char * logBuffer, const char * prefix, size_t maxSize)
{
    VerifyOrDie(logBuffer != nullptr);
    VerifyOrDie(prefix != nullptr);
    VerifyOrDie(maxSize > kTimeStampStringSize + strlen(prefix)); // Greater than to at least accommodate a ending Null Character

    // Derive the hours, minutes, seconds and milliseconds since boot time millisecond counter
    uint64_t bootTime   = chip::System::SystemClock().GetMonotonicMilliseconds64().count();
    size_t timestampLen = FormatTimestamp(logBuffer, maxSize, bootTime);
    if (timestampLen >= maxSize)
    {
        return 0; // Likely a snprintf error
    }
    return snprintf(logBuffer + timestampLen, maxSize - timestampLen, "%s", prefix);
}
size_t FormatTimestamp(char * buffer, size_t maxSize, uint64_t timestampMillis)
{
    VerifyOrDie(buffer != nullptr);
    VerifyOrDie(maxSize >= kTimeStampStringSize); // Greater than to at least accommodate a ending Null Character

    // Derive the hours, minutes, seconds and milliseconds since boot time millisecond counter
    uint16_t milliseconds = timestampMillis % 1000;
    uint32_t totalSeconds = timestampMillis / 1000;
    uint8_t seconds       = totalSeconds % 60;
    totalSeconds /= 60;
    uint8_t minutes = totalSeconds % 60;
    uint32_t hours  = totalSeconds / 60;

    return snprintf(buffer, maxSize, "[%04lu:%02u:%02u.%03u]", hours, minutes, seconds, milliseconds);
}

void HandleLog(const char * module, LogCategory category, const char * aFormat, va_list v)
{
    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];

    static_assert(sizeof(formattedMsg) >
                  kTimeStampStringSize + kMaxCategoryStrLen); // Greater than to at least accommodate a ending Null Character

    size_t prefixLen = 0;
    size_t moduleLen = strlen(module);
    if ((moduleLen > 0) && (moduleLen < CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE))
    {
        // Prepend module name if available
        prefixLen = snprintf(formattedMsg, sizeof(formattedMsg), "[%s] ", module);
    }

#if !SILABS_LOG_OUT_UART
    prefixLen += chip::Logging::Platform::AddTimeStampAndPrefixStr(
        formattedMsg, reinterpret_cast<const char *>(GetCategoryString(category)), CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE);
#endif // SILABS_LOG_OUT_UART
    if (prefixLen >= sizeof formattedMsg)
    {
        prefixLen = sizeof formattedMsg - 1; // prevent overflow
    }

    size_t len = vsnprintf(formattedMsg + prefixLen, sizeof formattedMsg - prefixLen, aFormat, v);

    if (len >= sizeof formattedMsg - prefixLen)
    {
        formattedMsg[sizeof formattedMsg - 1] = '\0';
    }

#if SILABS_LOG_OUT_UART
    // Silabs UART Log trunc
    uint8_t messageLen = len + prefixLen > 255 ? 255 : static_cast<uint8_t>(len + prefixLen);
    uartLogWrite(formattedMsg, messageLen, category, chip::System::SystemClock().GetMonotonicMilliseconds64().count());
#else
    PrintLog(formattedMsg);
#endif // SILABS_LOG_OUT_UART

    // Let the application know that a log message has been emitted.
    chip::DeviceLayer::OnLogOutput();
}

} // namespace Platform
} // namespace Logging
} // namespace chip

#if !SILABS_LOG_OUT_UART
/**
 * Print a log message
 */
static void PrintLog(const char * msg)
{
    if (sLogInitialized)
    {
        size_t sz;
        sz = strlen(msg);

#if PW_RPC_ENABLED
        PigweedLogger::putString(msg, sz);
#endif // PW_RPC_ENABLED
        SEGGER_RTT_WriteNoLock(LOG_RTT_BUFFER_INDEX, msg, sz);

#if PW_RPC_ENABLED
        const char * newline = "\r\n";
        sz                   = strlen(newline);
#if PW_RPC_ENABLED
        PigweedLogger::putString(newline, sz);
#endif // PW_RPC_ENABLED
        SEGGER_RTT_WriteNoLock(LOG_RTT_BUFFER_INDEX, newline, sz);
#endif
    }
}
#endif // !SILABS_LOG_OUT_UART
#endif // SILABS_LOG_ENABLED

/**
 * Initialize logging
 */
extern "C" void silabsInitLog(void)
{
#if SILABS_LOG_ENABLED
#if !SILABS_LOG_OUT_UART
#if LOG_RTT_BUFFER_INDEX != 0
    SEGGER_RTT_ConfigUpBuffer(LOG_RTT_BUFFER_INDEX, LOG_RTT_BUFFER_NAME, sLogBuffer, LOG_RTT_BUFFER_SIZE,
                              SEGGER_RTT_MODE_NO_BLOCK_TRIM);

    SEGGER_RTT_ConfigDownBuffer(LOG_RTT_BUFFER_INDEX, LOG_RTT_BUFFER_NAME, sCmdLineBuffer, LOG_RTT_BUFFER_SIZE,
                                SEGGER_RTT_MODE_NO_BLOCK_SKIP);
#else
    SEGGER_RTT_SetFlagsUpBuffer(LOG_RTT_BUFFER_INDEX, SEGGER_RTT_MODE_NO_BLOCK_TRIM);
#endif
#endif // !SILABS_LOG_OUT_UART

#ifdef PW_RPC_ENABLED
    PigweedLogger::init();
#endif
    sLogInitialized = true;
#endif // SILABS_LOG_ENABLED
}

/**
 * General-purpose logging function
 */
extern "C" void silabsLog(const char * aFormat, ...)
{
    va_list v;

    va_start(v, aFormat);
#if SILABS_LOG_ENABLED
    chip::Logging::Platform::HandleLog(reinterpret_cast<const char *>(kLogNone), kLog_Silabs, aFormat, v);
#endif // SILABS_LOG_ENABLED

    va_end(v);
}

namespace chip {
namespace Logging {
namespace Platform {

/**
 * CHIP log output functions.
 */
void LogV(const char * module, uint8_t category, const char * aFormat, va_list v)
{
#if SILABS_LOG_ENABLED && _CHIP_USE_LOGGING
    if (IsCategoryEnabled(category))
    {
        LogCategory categoryEnum = kLog_None;

        switch (category)
        {
        case kLogCategory_Error:
            categoryEnum = kLog_Error;
            break;
        case kLogCategory_Detail:
            categoryEnum = kLog_Detail;
            break;
        case kLogCategory_Progress:
        default:
            categoryEnum = kLog_Progress;
            break;
        }
        chip::Logging::Platform::HandleLog(module, categoryEnum, aFormat, v);
    }

    // Let the application know that a log message has been emitted.
    chip::DeviceLayer::OnLogOutput();
#endif // SILABS_LOG_ENABLED && _CHIP_USE_LOGGING
}

} // namespace Platform
} // namespace Logging
} // namespace chip

#if CHIP_SYSTEM_CONFIG_USE_LWIP

/**
 * LwIP log output function.
 */
extern "C" void LwIPLog(const char * aFormat, ...)
{
    va_list v;
    va_start(v, aFormat);
#if SILABS_LOG_ENABLED
    chip::Logging::Platform::HandleLog(reinterpret_cast<const char *>(kLogLwip), kLog_Lwip, aFormat, v);
    chip::DeviceLayer::OnLogOutput();
#endif // SILABS_LOG_ENABLED
    va_end(v);
}
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
/**
 * Platform logging function for OpenThread
 */
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
extern "C" void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char * aFormat, ...)
{
    (void) aLogRegion;
    va_list v;

    va_start(v, aFormat);
#if SILABS_LOG_ENABLED
    LogCategory category = kLog_Silabs;
    if (sLogInitialized)
    {
        switch (aLogLevel)
        {
        case OT_LOG_LEVEL_CRIT:
            category = kLog_Error;
            break;
        case OT_LOG_LEVEL_WARN:
            category = kLog_Warning;
            break;
        case OT_LOG_LEVEL_NOTE:
        case OT_LOG_LEVEL_INFO:
            category = kLog_Progress;
            break;
        case OT_LOG_LEVEL_DEBG:
        default:
            category = kLog_Detail;
            break;
        }
        chip::Logging::Platform::HandleLog(reinterpret_cast<const char *>(kOTModule), category, aFormat, v);
    }

    // Let the application know that a log message has been emitted.
    chip::DeviceLayer::OnLogOutput();
#endif // SILABS_LOG_ENABLED
    va_end(v);
}
#endif // CHIP_ENABLE_OPENTHREAD
