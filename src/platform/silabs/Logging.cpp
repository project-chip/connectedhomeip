/* See Project CHIP LICENSE file for licensing information. */
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

#ifndef BRD4325A

#ifdef RAIL_ASSERT_DEBUG_STRING
#include "rail_assert_error_codes.h"
#endif
#endif // BRD4325A

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

// Enable RTT by default
#ifndef SILABS_LOG_OUT_RTT
#define SILABS_LOG_OUT_RTT 1
#endif

// SEGGER_RTT includes
#if SILABS_LOG_OUT_RTT
#include "SEGGER_RTT.h"
#include "SEGGER_RTT_Conf.h"
#endif

#define LOG_ERROR "[error ]"
#define LOG_WARN "[warn  ]"
#define LOG_INFO "[info  ]"
#define LOG_DETAIL "[detail]"
#if CHIP_SYSTEM_CONFIG_USE_LWIP
#define LOG_LWIP "[lwip  ]"
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
#define LOG_SILABS "[silabs ]"
// If a new category string LOG_* is created, add it in the MaxStringLength arguments below
#if CHIP_SYSTEM_CONFIG_USE_LWIP
static constexpr size_t kMaxCategoryStrLen = chip::MaxStringLength(LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DETAIL, LOG_LWIP, LOG_SILABS);
#else
static constexpr size_t kMaxCategoryStrLen = chip::MaxStringLength(LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DETAIL, LOG_SILABS);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

static constexpr size_t kMaxTimestampStrLen = 16; // "[" (HH)HH:MM:SS + "." + miliseconds(3digits) + "]"

#if SILABS_LOG_ENABLED
static bool sLogInitialized = false;
#endif
#if LOG_RTT_BUFFER_INDEX != 0
static uint8_t sLogBuffer[LOG_RTT_BUFFER_SIZE];
static uint8_t sCmdLineBuffer[LOG_RTT_BUFFER_SIZE];
#endif

#if SILABS_LOG_ENABLED

using namespace chip;

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
    VerifyOrDie(logBuffer != nullptr);
    VerifyOrDie(prefix != nullptr);
    VerifyOrDie(maxSize > kMaxTimestampStrLen + strlen(prefix)); // Greater than to at least accommodate a ending Null Character

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
 * Print a log message to RTT
 */
static void PrintLog(const char * msg)
{
    if (sLogInitialized)
    {
        size_t sz;
        sz = strlen(msg);

#if SILABS_LOG_OUT_UART
        uartLogWrite(msg, sz);
#elif PW_RPC_ENABLED
        PigweedLogger::putString(msg, sz);
#else
        SEGGER_RTT_WriteNoLock(LOG_RTT_BUFFER_INDEX, msg, sz);
#endif // SILABS_LOG_OUT_UART

#if SILABS_LOG_OUT_RTT || PW_RPC_ENABLED
        const char * newline = "\r\n";
        sz                   = strlen(newline);
#if PW_RPC_ENABLED
        PigweedLogger::putString(newline, sz);
#else
        SEGGER_RTT_WriteNoLock(LOG_RTT_BUFFER_INDEX, newline, sz);
#endif // PW_RPC_ENABLED
#endif
    }
}
#endif // SILABS_LOG_ENABLED

/**
 * Initialize Segger RTT for logging
 */
extern "C" void silabsInitLog(void)
{
#if SILABS_LOG_ENABLED
#if SILABS_LOG_OUT_RTT
#if LOG_RTT_BUFFER_INDEX != 0
    SEGGER_RTT_ConfigUpBuffer(LOG_RTT_BUFFER_INDEX, LOG_RTT_BUFFER_NAME, sLogBuffer, LOG_RTT_BUFFER_SIZE,
                              SEGGER_RTT_MODE_NO_BLOCK_TRIM);

    SEGGER_RTT_ConfigDownBuffer(LOG_RTT_BUFFER_INDEX, LOG_RTT_BUFFER_NAME, sCmdLineBuffer, LOG_RTT_BUFFER_SIZE,
                                SEGGER_RTT_MODE_NO_BLOCK_SKIP);
#else
    SEGGER_RTT_SetFlagsUpBuffer(LOG_RTT_BUFFER_INDEX, SEGGER_RTT_MODE_NO_BLOCK_TRIM);
#endif
#endif // SILABS_LOG_OUT_RTT

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
    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
    static_assert(sizeof(formattedMsg) >
                  kMaxTimestampStrLen + kMaxCategoryStrLen); // Greater than to at least accommodate a ending Null Character

    size_t prefixLen = AddTimeStampAndPrefixStr(formattedMsg, LOG_SILABS, CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE);
    size_t len       = vsnprintf(formattedMsg + prefixLen, sizeof formattedMsg - prefixLen, aFormat, v);

    if (len >= sizeof formattedMsg - prefixLen)
    {
        formattedMsg[sizeof formattedMsg - 1] = '\0';
    }

    PrintLog(formattedMsg);
#endif // SILABS_LOG_ENABLED

    va_end(v);
}

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

/**
 * CHIP log output functions.
 */
void LogV(const char * module, uint8_t category, const char * aFormat, va_list v)
{
#if SILABS_LOG_ENABLED && _CHIP_USE_LOGGING
    if (IsCategoryEnabled(category))
    {
        char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
        size_t formattedMsgLen;

        // len for Category string + "[" + Module name + "] " (Brackets and space =3)
        constexpr size_t maxPrefixLen = kMaxTimestampStrLen + kMaxCategoryStrLen + chip::Logging::kMaxModuleNameLen + 3;
        static_assert(sizeof(formattedMsg) > maxPrefixLen); // Greater than to at least accommodate a ending Null Character

        switch (category)
        {
        case kLogCategory_Error:
            formattedMsgLen = AddTimeStampAndPrefixStr(formattedMsg, LOG_ERROR, CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE);
            break;
        case kLogCategory_Progress:
        default:
            formattedMsgLen = AddTimeStampAndPrefixStr(formattedMsg, LOG_INFO, CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE);
            break;
        case kLogCategory_Detail:
            formattedMsgLen = AddTimeStampAndPrefixStr(formattedMsg, LOG_DETAIL, CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE);
            break;
        }

        // Add the module name to the log prefix , e.g. "[DL] "
        snprintf(formattedMsg + formattedMsgLen, sizeof(formattedMsg) - formattedMsgLen, "[%s] ", module);
        formattedMsg[sizeof(formattedMsg) - 1] = 0;
        formattedMsgLen                        = strlen(formattedMsg);

        size_t len = vsnprintf(formattedMsg + formattedMsgLen, sizeof formattedMsg - formattedMsgLen, aFormat, v);

        if (len >= sizeof formattedMsg - formattedMsgLen)
        {
            formattedMsg[sizeof formattedMsg - 1] = '\0';
        }

        PrintLog(formattedMsg);
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
    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];

    size_t prefixLen = AddTimeStampAndPrefixStr(formattedMsg, LOG_LWIP, CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE);
    size_t len       = vsnprintf(formattedMsg + prefixLen, sizeof formattedMsg - prefixLen, aFormat, v);

    if (len >= sizeof formattedMsg - prefixLen)
    {
        formattedMsg[sizeof formattedMsg - 1] = '\0';
    }

    PrintLog(formattedMsg);

#if configCHECK_FOR_STACK_OVERFLOW
    // Force a stack overflow check.
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
        taskYIELD();
#endif

    // Let the application know that a log message has been emitted.
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
    size_t prefixLen;
    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];

    if (sLogInitialized)
    {
        switch (aLogLevel)
        {
        case OT_LOG_LEVEL_CRIT:
            prefixLen = AddTimeStampAndPrefixStr(formattedMsg, LOG_ERROR "[ot] ", CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE);
            break;
        case OT_LOG_LEVEL_WARN:
            prefixLen = AddTimeStampAndPrefixStr(formattedMsg, LOG_WARN "[ot] ", CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE);
            break;
        case OT_LOG_LEVEL_NOTE:
        case OT_LOG_LEVEL_INFO:
            prefixLen = AddTimeStampAndPrefixStr(formattedMsg, LOG_INFO "[ot] ", CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE);
            break;
        case OT_LOG_LEVEL_DEBG:
        default:
            prefixLen = AddTimeStampAndPrefixStr(formattedMsg, LOG_DETAIL "[ot] ", CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE);
            break;
        }

        size_t len = vsnprintf(formattedMsg + prefixLen, sizeof(formattedMsg) - prefixLen, aFormat, v);

        if (len >= sizeof formattedMsg - prefixLen)
        {
            formattedMsg[sizeof formattedMsg - 1] = '\0';
        }

        PrintLog(formattedMsg);

#if configCHECK_FOR_STACK_OVERFLOW
        // Force a stack overflow check.
        if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
            taskYIELD();
#endif
    }

    // Let the application know that a log message has been emitted.
    chip::DeviceLayer::OnLogOutput();
#endif // SILABS_LOG_ENABLED
    va_end(v);
}
#endif // CHIP_ENABLE_OPENTHREAD
