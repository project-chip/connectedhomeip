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

#include "AppConfig.h"
#include <FreeRTOS.h>
#include <queue.h>
#include <stdio.h>
#include <string.h>
#include <task.h>

#ifndef BRD4325A
#include "rail_types.h"

#ifdef RAIL_ASSERT_DEBUG_STRING
#include "rail_assert_error_codes.h"
#endif
#endif // BRD4325A

#ifdef BRD4325A // For SiWx917 Platform only
#include "core_cm4.h"
#endif

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
#endif

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
extern "C" void efr32Log(const char * aFormat, ...)
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

#if HARD_FAULT_LOG_ENABLE && SILABS_LOG_ENABLED

/**
 * Log register contents to UART when a hard fault occurs.
 */
extern "C" void debugHardfault(uint32_t * sp)
{
    uint32_t cfsr  = SCB->CFSR;
    uint32_t hfsr  = SCB->HFSR;
    uint32_t mmfar = SCB->MMFAR;
    uint32_t bfar  = SCB->BFAR;
    uint32_t r0    = sp[0];
    uint32_t r1    = sp[1];
    uint32_t r2    = sp[2];
    uint32_t r3    = sp[3];
    uint32_t r12   = sp[4];
    uint32_t lr    = sp[5];
    uint32_t pc    = sp[6];
    uint32_t psr   = sp[7];
    char formattedMsg[32];

    if (sLogInitialized == false)
    {
        silabsInitLog();
    }

    snprintf(formattedMsg, sizeof formattedMsg, LOG_ERROR "HardFault:\n");
    PrintLog(formattedMsg);
    snprintf(formattedMsg, sizeof formattedMsg, "SCB->CFSR   0x%08lx", cfsr);
    PrintLog(formattedMsg);
    snprintf(formattedMsg, sizeof formattedMsg, "SCB->HFSR   0x%08lx", hfsr);
    PrintLog(formattedMsg);
    snprintf(formattedMsg, sizeof formattedMsg, "SCB->MMFAR  0x%08lx", mmfar);
    PrintLog(formattedMsg);
    snprintf(formattedMsg, sizeof formattedMsg, "SCB->BFAR   0x%08lx", bfar);
    PrintLog(formattedMsg);
    snprintf(formattedMsg, sizeof formattedMsg, "SP          0x%08lx", (uint32_t) sp);
    PrintLog(formattedMsg);
    snprintf(formattedMsg, sizeof formattedMsg, "R0          0x%08lx\n", r0);
    PrintLog(formattedMsg);
    snprintf(formattedMsg, sizeof formattedMsg, "R1          0x%08lx\n", r1);
    PrintLog(formattedMsg);
    snprintf(formattedMsg, sizeof formattedMsg, "R2          0x%08lx\n", r2);
    PrintLog(formattedMsg);
    snprintf(formattedMsg, sizeof formattedMsg, "R3          0x%08lx\n", r3);
    PrintLog(formattedMsg);
    snprintf(formattedMsg, sizeof formattedMsg, "R12         0x%08lx\n", r12);
    PrintLog(formattedMsg);
    snprintf(formattedMsg, sizeof formattedMsg, "LR          0x%08lx\n", lr);
    PrintLog(formattedMsg);
    snprintf(formattedMsg, sizeof formattedMsg, "PC          0x%08lx\n", pc);
    PrintLog(formattedMsg);
    snprintf(formattedMsg, sizeof formattedMsg, "PSR         0x%08lx\n", psr);
    PrintLog(formattedMsg);

    while (true)
        ;
}

/**
 * Override default hard-fault handler
 */
extern "C" __attribute__((naked)) void HardFault_Handler(void)
{
    __asm volatile("tst lr, #4                                    \n"
                   "ite eq                                        \n"
                   "mrseq r0, msp                                 \n"
                   "mrsne r0, psp                                 \n"
                   "ldr r1, debugHardfault_address                \n"
                   "bx r1                                         \n"
                   "debugHardfault_address: .word debugHardfault  \n");
}
#endif // HARD_FAULT_LOG_ENABLE && SILABS_LOG_ENABLED

#if HARD_FAULT_LOG_ENABLE
extern "C" void vApplicationMallocFailedHook(void)
{
    /* Called if a call to pvPortMalloc() fails because there is insufficient
    free memory available in the FreeRTOS heap.  pvPortMalloc() is called
    internally by FreeRTOS API functions that create tasks, queues, software
    timers, and semaphores.  The size of the FreeRTOS heap is set by the
    configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */

#if SILABS_LOG_ENABLED
    SILABS_LOG("Failed do a malloc on HEAP. Is it too small ?");
#endif
    /* Force an assert. */
    configASSERT((volatile void *) NULL);
}
/*-----------------------------------------------------------*/

extern "C" void vApplicationStackOverflowHook(TaskHandle_t pxTask, char * pcTaskName)
{
    //(void) pcTaskName;
    (void) pxTask;

    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */

#if SILABS_LOG_ENABLED
    SILABS_LOG("TASK OVERFLOW");
    SILABS_LOG(pcTaskName);
#endif
    /* Force an assert. */
    configASSERT((volatile void *) NULL);
}

extern "C" void vApplicationTickHook(void) {}

/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
extern "C" void vApplicationGetIdleTaskMemory(StaticTask_t ** ppxIdleTaskTCBBuffer, StackType_t ** ppxIdleTaskStackBuffer,
                                              uint32_t * pulIdleTaskStackSize)
{
    /* If the buffers to be provided to the Idle task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
extern "C" void vApplicationGetTimerTaskMemory(StaticTask_t ** ppxTimerTaskTCBBuffer, StackType_t ** ppxTimerTaskStackBuffer,
                                               uint32_t * pulTimerTaskStackSize)
{
    /* If the buffers to be provided to the Timer task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

#ifndef BRD4325A
extern "C" void RAILCb_AssertFailed(RAIL_Handle_t railHandle, uint32_t errorCode)
{
#if SILABS_LOG_ENABLED
#ifdef RAIL_ASSERT_DEBUG_STRING
    static const char * railErrorMessages[] = RAIL_ASSERT_ERROR_MESSAGES;
    const char * errorMessage               = "Unknown";

    If this error code is within the range of known error messages then use the appropriate error
        message.if (errorCode < (sizeof(railErrorMessages) / sizeof(char *)))
    {
        errorMessage = railErrorMessages[errorCode];
    }
    SILABS_LOG("RAIL Assert : %s", errorMessage);
#else
    SILABS_LOG("RAIL Assert : %ld", errorCode);
#endif // RAIL_ASSERT_DEBUG_STRING
#endif // SILABS_LOG_ENABLED
    while (1)
        ;
}
#endif // BRD4325A

#endif // HARD_FAULT_LOG_ENABLE
