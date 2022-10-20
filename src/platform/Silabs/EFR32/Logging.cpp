/* See Project CHIP LICENSE file for licensing information. */
#include <platform/logging/LogV.h>

#include <lib/core/CHIPConfig.h>
#include <platform/CHIPDeviceConfig.h>

#include <lib/support/SafeString.h>
#include <lib/support/logging/CHIPLogging.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <openthread/platform/logging.h>
#endif

#include "AppConfig.h"
#include <FreeRTOS.h>
#include <queue.h>
#include <stdio.h>
#include <string.h>
#include <task.h>

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

// FreeRTOS includes
#include "SEGGER_RTT.h"
#include "SEGGER_RTT_Conf.h"

#define LOG_ERROR "<error > "
#define LOG_WARN "<warn  > "
#define LOG_INFO "<info  > "
#define LOG_DETAIL "<detail> "
#if CHIP_SYSTEM_CONFIG_USE_LWIP
#define LOG_LWIP "<lwip  > "
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
#define LOG_EFR32 "<efr32 > "
// If a new category string LOG_* is created, add it in the MaxStringLength arguments below
#if CHIP_SYSTEM_CONFIG_USE_LWIP
static constexpr size_t kMaxCategoryStrLen = chip::MaxStringLength(LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DETAIL, LOG_LWIP, LOG_EFR32);
#else
static constexpr size_t kMaxCategoryStrLen = chip::MaxStringLength(LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DETAIL, LOG_EFR32);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if EFR32_LOG_ENABLED
static bool sLogInitialized = false;
#endif
#if LOG_RTT_BUFFER_INDEX != 0
static uint8_t sLogBuffer[LOG_RTT_BUFFER_SIZE];
static uint8_t sCmdLineBuffer[LOG_RTT_BUFFER_SIZE];
#endif

#if EFR32_LOG_ENABLED
/**
 * Print a log message to RTT
 */
static void PrintLog(const char * msg)
{
    if (sLogInitialized)
    {
        size_t sz;
        sz = strlen(msg);
        SEGGER_RTT_WriteNoLock(LOG_RTT_BUFFER_INDEX, msg, sz);
#ifdef PW_RPC_ENABLED
        PigweedLogger::putString(msg, sz);
#endif

        const char * newline = "\r\n";
        sz                   = strlen(newline);
        SEGGER_RTT_WriteNoLock(LOG_RTT_BUFFER_INDEX, newline, sz);
#ifdef PW_RPC_ENABLED
        PigweedLogger::putString(newline, sz);
#endif
    }
}
#endif // EFR32_LOG_ENABLED

/**
 * Initialize Segger RTT for logging
 */
extern "C" void efr32InitLog(void)
{
#if EFR32_LOG_ENABLED
#if LOG_RTT_BUFFER_INDEX != 0
    SEGGER_RTT_ConfigUpBuffer(LOG_RTT_BUFFER_INDEX, LOG_RTT_BUFFER_NAME, sLogBuffer, LOG_RTT_BUFFER_SIZE,
                              SEGGER_RTT_MODE_NO_BLOCK_TRIM);

    SEGGER_RTT_ConfigDownBuffer(LOG_RTT_BUFFER_INDEX, LOG_RTT_BUFFER_NAME, sCmdLineBuffer, LOG_RTT_BUFFER_SIZE,
                                SEGGER_RTT_MODE_NO_BLOCK_SKIP);
#else
    SEGGER_RTT_SetFlagsUpBuffer(LOG_RTT_BUFFER_INDEX, SEGGER_RTT_MODE_NO_BLOCK_TRIM);
#endif

#ifdef PW_RPC_ENABLED
    PigweedLogger::init();
#endif
    sLogInitialized = true;
#endif // EFR32_LOG_ENABLED
}

/**
 * General-purpose logging function
 */
extern "C" void efr32Log(const char * aFormat, ...)
{
    va_list v;

    va_start(v, aFormat);
#if EFR32_LOG_ENABLED
    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
    static_assert(sizeof(formattedMsg) > kMaxCategoryStrLen); // Greater than to at least accommodate a ending Null Character

    strcpy(formattedMsg, LOG_EFR32);
    size_t prefixLen = strlen(formattedMsg);
    size_t len       = vsnprintf(formattedMsg + prefixLen, sizeof formattedMsg - prefixLen, aFormat, v);

    if (len >= sizeof formattedMsg - prefixLen)
    {
        formattedMsg[sizeof formattedMsg - 1] = '\0';
    }

    PrintLog(formattedMsg);
#endif // EFR32_LOG_ENABLED

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
#if EFR32_LOG_ENABLED && _CHIP_USE_LOGGING
    if (IsCategoryEnabled(category))
    {
        char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
        size_t formattedMsgLen;

        // len for Category string + "[" + Module name + "] " (Brackets and space =3)
        constexpr size_t maxPrefixLen = kMaxCategoryStrLen + chip::Logging::kMaxModuleNameLen + 3;
        static_assert(sizeof(formattedMsg) > maxPrefixLen); // Greater than to at least accommodate a ending Null Character

        switch (category)
        {
        case kLogCategory_Error:
            strcpy(formattedMsg, LOG_ERROR);
            break;
        case kLogCategory_Progress:
        default:
            strcpy(formattedMsg, LOG_INFO);
            break;
        case kLogCategory_Detail:
            strcpy(formattedMsg, LOG_DETAIL);
            break;
        }

        formattedMsgLen = strlen(formattedMsg);

        // Form the log prefix, e.g. "[DL] "
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
#endif // EFR32_LOG_ENABLED && _CHIP_USE_LOGGING
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
#if EFR32_LOG_ENABLED
    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];

    strcpy(formattedMsg, LOG_LWIP);
    size_t prefixLen = strlen(formattedMsg);
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
#endif // EFR32_LOG_ENABLED
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
#if EFR32_LOG_ENABLED
    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];

    if (sLogInitialized)
    {
        switch (aLogLevel)
        {
        case OT_LOG_LEVEL_CRIT:
            strcpy(formattedMsg, LOG_ERROR "[ot] ");
            break;
        case OT_LOG_LEVEL_WARN:
            strcpy(formattedMsg, LOG_WARN "[ot] ");
            break;
        case OT_LOG_LEVEL_NOTE:
            strcpy(formattedMsg, LOG_INFO "[ot] ");
            break;
        case OT_LOG_LEVEL_INFO:
            strcpy(formattedMsg, LOG_INFO "[ot] ");
            break;
        case OT_LOG_LEVEL_DEBG:
            strcpy(formattedMsg, LOG_DETAIL "[ot] ");
            break;
        default:
            strcpy(formattedMsg, LOG_DETAIL "[ot] ");
            break;
        }

        size_t prefixLen = strlen(formattedMsg);
        size_t len       = vsnprintf(formattedMsg + prefixLen, sizeof(formattedMsg) - prefixLen, aFormat, v);

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
#endif // EFR32_LOG_ENABLED
    va_end(v);
}
#endif // CHIP_ENABLE_OPENTHREAD

#if HARD_FAULT_LOG_ENABLE && EFR32_LOG_ENABLED

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
        efr32InitLog();
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

extern "C" void vApplicationMallocFailedHook(void)
{
    /* Called if a call to pvPortMalloc() fails because there is insufficient
    free memory available in the FreeRTOS heap.  pvPortMalloc() is called
    internally by FreeRTOS API functions that create tasks, queues, software
    timers, and semaphores.  The size of the FreeRTOS heap is set by the
    configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */

    EFR32_LOG("Failed do a malloc on HEAP. Is it too small ?");

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
    EFR32_LOG("TASK OVERFLOW");
    EFR32_LOG(pcTaskName);
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
#endif // HARD_FAULT_LOG_ENABLE && EFR32_LOG_ENABLED
