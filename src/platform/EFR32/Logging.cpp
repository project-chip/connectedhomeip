/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 *          Provides implementations for the CHIP and LwIP logging
 *          functions on Silicon Labs EFR32 platforms.
 *
 *          Logging should be initialized by a call to efr32LogInit().  A
 *          spooler task is created that sends the logs to the UART.  Log
 *          entries are queued. If the queue is full then by default error
 *          logs wait indefinitely until a slot is available whereas
 *          non-error logs are dropped to avoid delays.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include <queue.h>
#include <retargetserial.h>
#include <stdio.h>
#include <task.h>

// RTT Buffer size and name
#ifndef LOG_RTT_BUFFER_INDEX
#define LOG_RTT_BUFFER_INDEX 0
#endif

#ifndef LOG_RTT_BUFFER_NAME
#define LOG_RTT_BUFFER_NAME "Terminal"
#endif

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
#define LOG_LWIP "<lwip  > "
#define LOG_EFR32 "<efr32 > "

using namespace ::chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;

static bool sLogInitialized = false;
static uint8_t sLogBuffer[LOG_RTT_BUFFER_SIZE];

/**
 * Print a log message to RTT
 */
static void PrintLog(const char * msg)
{
#if EFR32_LOG_ENABLED
    if (sLogInitialized)
    {
        size_t sz;
        sz = strlen(msg);
        SEGGER_RTT_WriteNoLock(0, msg, sz);

        const char * newline = "\r\n";
        sz                   = strlen(newline);
        SEGGER_RTT_WriteNoLock(0, newline, sz);
    }
#endif // EFR32_LOG_ENABLED
}

/**
 * Initialize Segger RTT for logging
 */
extern "C" void efr32LogInit(void)
{
#if EFR32_LOG_ENABLED
    SEGGER_RTT_ConfigUpBuffer(LOG_RTT_BUFFER_INDEX, LOG_RTT_BUFFER_NAME, sLogBuffer, LOG_RTT_BUFFER_SIZE,
                              SEGGER_RTT_MODE_NO_BLOCK_TRIM);
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
    char formattedMsg[CHIP_DEVICE_CONFIG_LOG_MESSAGE_MAX_SIZE];

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

/**
 * CHIP log output functions.
 */
void LogV(uint8_t module, uint8_t category, const char * aFormat, va_list v)
{
#if EFR32_LOG_ENABLED && _CHIP_USE_LOGGING
    if (IsCategoryEnabled(category))
    {
        char formattedMsg[CHIP_DEVICE_CONFIG_LOG_MESSAGE_MAX_SIZE];
        size_t formattedMsgLen;

        constexpr size_t maxPrefixLen = ChipLoggingModuleNameLen + 3;
        static_assert(sizeof(formattedMsg) > maxPrefixLen);

        switch (category)
        {
        case kLogCategory_Error:
            strcpy(formattedMsg, LOG_ERROR);
            break;
        case kLogCategory_Progress:
        case kLogCategory_Retain:
        default:
            strcpy(formattedMsg, LOG_INFO);
            break;
        case kLogCategory_Detail:
            strcpy(formattedMsg, LOG_DETAIL);
            break;
        }

        formattedMsgLen = strlen(formattedMsg);

        // Form the log prefix, e.g. "[DL] "
        formattedMsg[formattedMsgLen++] = '[';
        GetModuleName(formattedMsg + formattedMsgLen, module);
        formattedMsgLen                 = strlen(formattedMsg);
        formattedMsg[formattedMsgLen++] = ']';
        formattedMsg[formattedMsgLen++] = ' ';

        size_t len = vsnprintf(formattedMsg + formattedMsgLen, sizeof formattedMsg - formattedMsgLen, aFormat, v);

        if (len >= sizeof formattedMsg - formattedMsgLen)
        {
            formattedMsg[sizeof formattedMsg - 1] = '\0';
        }

        PrintLog(formattedMsg);
    }

    // Let the application know that a log message has been emitted.
    DeviceLayer::OnLogOutput();
#endif // EFR32_LOG_ENABLED && _CHIP_USE_LOGGING
}

} // namespace Logging
} // namespace chip

/**
 * LwIP log output function.
 */
extern "C" void LwIPLog(const char * aFormat, ...)
{
    va_list v;

    va_start(v, aFormat);
#if EFR32_LOG_ENABLED
    char formattedMsg[CHIP_DEVICE_CONFIG_LOG_MESSAGE_MAX_SIZE];

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
    DeviceLayer::OnLogOutput();
#endif // EFR32_LOG_ENABLED
    va_end(v);
}

/**
 * Platform logging function for OpenThread
 */
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
extern "C" void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char * aFormat, ...)
{
    IgnoreUnusedVariable(aLogRegion);
    va_list v;

    va_start(v, aFormat);
#if EFR32_LOG_ENABLED
    char formattedMsg[CHIP_DEVICE_CONFIG_LOG_MESSAGE_MAX_SIZE];

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
    DeviceLayer::OnLogOutput();
#endif // EFR32_LOG_ENABLED
    va_end(v);
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

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
        efr32LogInit();
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

    while (1)
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

#endif // HARD_FAULT_LOG_ENABLE && EFR32_LOG_ENABLED
